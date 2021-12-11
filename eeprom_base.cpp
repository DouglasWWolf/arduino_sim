#include <string.h>
#include "eeprom_base.h"
#include "crc32.h"

// Our magic-number that indicates our structure exists.  In ASCII "AADW" ;-)
#define MAGIC_NUMBER (uint32_t)0x41414457

// Our data header is the first 16 bytes of the data structure
#define m_header (*(header_t*)m_data.ptr)

// Adds a constant to a void ptr
#define add_ptr(x,y) (void*)(((char*)x)+y)

//=========================================================================================================
// Constructor() - Saves wear-leveling setup information and initializes our internal data-descriptor
//=========================================================================================================
CEEPROM_Base::CEEPROM_Base()
{
    // Set up default wear-leveling parameters (i.e., no wear-leveling)
    m_wl = { 1, 0, nullptr };

    // By default we perform "dirty checking" on the data structure prior to writing it to physical EEPROM
    m_is_dirty_checking = true;

    // The EEPROM data structure in RAM is not yet dirty
    m_is_dirty = false;

    // Assume for the moment that we won't be doing wear-level caching
    m_is_caching = false;

    // Just for good measure, clear the error status
    m_error = error_t::OK;
}
//=========================================================================================================



//=========================================================================================================
// read() - Reads the physical EEPROM into our structure
//=========================================================================================================
bool CEEPROM_Base::read()
{
    uint16_t address;

    // Presume for a moment that this routine is going to succeed
    m_error = error_t::OK;
        
    // Ensure that the wear-leveling slots are large enough to hold our data structure!!
    if (bug_check()) return false;

    // Our main data structure always defaults to all zeros.  This will ensure that if our
    // structure in RAM is longer than the structure in EEPROM, the new fields in RAM will
    // be initialized to zero
    memset(m_data.ptr, 0, m_data.length);

    // This is the length of the data structure sans header in both EEPROM and RAM
    const uint16_t eeprom_data_only_length = m_header.data_len - sizeof header_t;
    const uint16_t    ram_data_only_length = m_data.length     - sizeof header_t;

    // Fetch the header for the most recent edition of our structure that exists in EEPROM
    if (!find_most_recent_edition(&m_header, &address))
    {
        m_error = error_t::IO;
    }

    // If a valid edition header was found, read in the main data
    if (m_header.magic == MAGIC_NUMBER)
    {
        // We want to read in every byte of the data structure in EEPROM
        uint16_t read_length = eeprom_data_only_length;

        // Make sure it doesn't overflow our data structure in RAM!
        if (read_length > ram_data_only_length) read_length = ram_data_only_length;

        // This is where our data structure starts in RAM
        void* ram_data = add_ptr(m_data.ptr, sizeof header_t);
       
        // Read the data structure from EEPROM into RAM
        if (!read_physical_block(ram_data,  address + sizeof header_t, read_length))
        {
            m_error = error_t::IO;
        }

        // If there are no errors so far, check to see if the data we read was corrupted
        if (m_error == error_t::OK && m_header.crc != compute_crc(m_header.data_len))
        {
             m_error = error_t::CRC;
        }
    }
   
    // And we need to initialize any new fields that may be present in the data structure
    initialize_new_fields();

    // The data structure in RAM now matches the data structure in EEPROM
    mark_data_as_clean();

    // Tell the caller whether we were able to read the EEPROM
    return (m_error == error_t::OK);
}
//=========================================================================================================



//=========================================================================================================
// find_most_recent_edition() - Searches every wear-leveling slot in EEPROM and returns the header from
//                              the most recently written edition of our data
//
// On Exit: *p_address is valid only if *p_result contains a valid header
//
// Returns: true on success, false if an I/O error occurs
//=========================================================================================================
bool CEEPROM_Base::find_most_recent_edition(header_t* p_result, uint16_t* p_address)
{
    header_t  header;

    // If we don't find any edition of our data structure in EEPROM, we'll return an empty header
    memset(p_result, 0, sizeof header_t);

    // Loop through every possible slot
    for (int slot = 0; slot < m_wl.count; ++slot)
    {
        // Find the EEPROM address of this slot
        uint16_t address = slot_to_header_address(slot);

        // Fetch this slot's header from EEPROM
        if (!read_physical_block(&header, address, sizeof header)) return false;

        // If this is a valid header and is the the most recent edition so far, report it
        if (header.magic == MAGIC_NUMBER && header.edition > p_result->edition)
        {
            *p_result = header;
            *p_address = slot_to_header_address(slot);
        }
    }

    // Tell the caller all is well
    return true;
}
//=========================================================================================================



//=========================================================================================================
// find_least_recent_slot() - Searches every wear-leveling slot in EEPROM and returns the slot number
//                            of the least recently used slot
//
// Returns: true on success, false if an I/O error occurs
//=========================================================================================================
bool CEEPROM_Base::find_least_recent_address(uint16_t* p_address)
{
    header_t  header;
    uint32_t  oldest_edition = 0xFFFFFFFF;

    // If there's only one slot, its address is zero
    if (m_wl.count == 1)
    {
        *p_address = 0;
        return true;
    }

    // Loop through every possible slot
    for (int slot = 0; slot < m_wl.count; ++slot)
    {
        // Find the EEPROM address of this slot
        uint16_t address = slot_to_header_address(slot);

        // Fetch this slot's header from EEPROM
        if (!read_physical_block(&header, address, sizeof header)) return false;

        // If this slot is empty, hand the slot number to the caller
        if (header.magic != MAGIC_NUMBER)
        {
            *p_address = address;
            return true;
        }

        // If this header is for the oldest edition we've yet seen, record it
        if (header.edition < oldest_edition)
        {
            oldest_edition = header.edition;
            *p_address = address;
        }

    }

    // Tell the caller all is well
    return true;
}
//=========================================================================================================




//=========================================================================================================
// write() - If anything has changed in the data, writes the data (and a new header) to EEPROM
//=========================================================================================================
bool CEEPROM_Base::write(bool force_write)
{
    uint16_t address;

    // Presume for a moment that this routine is going to succeed
    m_error = error_t::OK;

    // Ensure that the wear-leveling slots are large enough to hold our data structure!!
    if (bug_check()) return false;

    // If we're not forcing the write, and the data isn't "dirty", don't commit it to EEPROM
    if (!force_write && !is_dirty()) return true;

    // Fill in all of the header fields
    m_header.magic    = MAGIC_NUMBER;
    m_header.data_len = m_data.length;
    m_header.format   = m_data.format;
    
    // We are about to write a new edition of the data to EEPROM
    ++m_header.edition;

    // Fill in the CRC of the header and data 
    m_header.crc = compute_crc(m_data.length);

    // Find the EEPROM address where this edition should be written
    if (!find_least_recent_address(&address))
    {
         m_error = error_t::IO;
         return false;
    }

    // Write the header and data structure to EEPROM
    if (!write_physical_block(m_data.ptr, address, m_data.length))
    {
        m_error = error_t::IO;
    }

    // The data structure in RAM now matches the data structure in EEPROM
    mark_data_as_clean();

    // If we get here, the write operation was a success
    return (m_error == error_t::OK);
}
//=========================================================================================================



//=========================================================================================================
// rollback() - Undo the most recent call to write()
//=========================================================================================================
bool CEEPROM_Base::roll_back()
{
    uint16_t address;

    // Presume for a moment that this routine is going to succeed
    m_error = error_t::OK;

    // Ensure that the wear-leveling slots are large enough to hold our data structure!!
    if (bug_check()) return false;

    // Fetch the header for the most recent edition of our structure that exists in EEPROM
    if (!find_most_recent_edition(&m_header, &address))
    {
        m_error = error_t::IO;
        return false;
    }

    // If we found an edition that can be rolled back...
    if (m_header.magic == MAGIC_NUMBER)
    {
        // Wipe it out in both RAM and EEPROM
        memset(&m_header, 0xFF, sizeof m_header);
        write_physical_block(&m_header, address, sizeof m_header);
    }

    // And read in the previous edition
    return read();
}
//=========================================================================================================




//=========================================================================================================
// destroy() - Destroys the header structure in EEPROM and in RAM
//=========================================================================================================
bool CEEPROM_Base::destroy()
{
    // Presume for the moment that this routine is going to succeed
    m_error = error_t::OK;

    // Ensure that the wear-leveling slots are large enough to hold our data structure!!
    if (bug_check()) return false;

    // Destroy our header in RAM
    memset(&m_header, 0xFF, sizeof m_header);

    // Loop through every slot in EEPROM...
    for (int slot = 0; slot < m_wl.count; ++slot)
    {
        // Compute the EEPROM address of this slot
        uint16_t address = slot_to_header_address(slot);

        // And destroy the header in this slot
        if (!write_physical_block(&m_header, address, sizeof m_header))
        {
            m_error = error_t::IO;
        }
    }


    // EEPROM has been destroyed.  Set up the appropriate structures in RAM
    memset(m_data.ptr, 0, m_data.length);
    initialize_new_fields();

    // The data structure in RAM now matches the data structure in EEPROM
    mark_data_as_clean();

    // Tell the caller whether this worked
    return (m_error == error_t::OK);
}
//=========================================================================================================



//=========================================================================================================
// compute_crc() - Computes a CRC32 of the combined header and data structures
//=========================================================================================================
uint32_t CEEPROM_Base::compute_crc(size_t data_length)
{
    // Save the existing CRC so we can restore it
    uint32_t old_crc = m_header.crc;

    // We don't want the CRC field to affect the CRC calculation
    m_header.crc = 0;

    // Compute the CRC
    uint32_t new_crc = crc32(m_data.ptr, m_data.length);

    // Restore the previous CRC
    m_header.crc = old_crc;

    // And hand the resulting CRC to the caller
    return new_crc;
}
//=========================================================================================================




//=========================================================================================================
// slot_to_header_address() - Converts a wear_leveling slot number to an EEPROM address
//=========================================================================================================
uint16_t CEEPROM_Base::slot_to_header_address(int slot)
{
    // If we're not doing wear leveling, everything always goes into slot 0
    if (m_wl.count == 1) return 0;

    // Return the EEPROM address of this slot
    return slot * m_wl.size;
}
//=========================================================================================================


//=========================================================================================================
// mark_data_as_clean() - Marks the data structure in RAM as "clean" (i.e, the data in EEPROM exactly
//                        matches the data in RAM)
//=========================================================================================================
void CEEPROM_Base::mark_data_as_clean()
{
    // If the derived class wants to do automatic dirty checking, save a clean copy of the data
    if (m_data.clean_copy) memcpy(m_data.clean_copy, m_data.ptr, m_data.length);

    // The data structure in RAM is no longer dirty
    m_is_dirty = false;
}
//=========================================================================================================


//=========================================================================================================
// is_dirty() - Determines whether the data structure in RAM is "dirty" (i.e., is it different than what
//              is currently in EEPROM?)
//=========================================================================================================
bool CEEPROM_Base::is_dirty()
{
    // If we're not doing "dirty checking", then we always assume the data is dirty
    if (!m_is_dirty_checking) return true;
    
    // If we're doing automatic "dirty checking", see if the data differs from the clean copy
    if (m_data.clean_copy && memcmp(m_data.ptr, m_data.clean_copy, m_data.length) != 0) return true;

    // Last but not least, find out if the derived class has told us the data is dirty
    return m_is_dirty;
}
//=========================================================================================================



//=========================================================================================================
// bug_check() - Ensures that the user's data structure will fit in a wear-leveling slot
//=========================================================================================================
bool CEEPROM_Base::bug_check()
{
    // Ensure that the wear-leveling slots are large enough to hold our data structure!!
    if (m_wl.count > 1 && m_wl.size < m_data.length)
    {
        m_error = error_t::BUG;
        return true;
    }

    // If we get here, we're bug-free  :-)
    return false;
}
//=========================================================================================================
