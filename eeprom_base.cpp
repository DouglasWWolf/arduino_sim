#include <string.h>
#include "eeprom_base.h"
#include "crc32.h"

// Our magic-number that indicates our structure exists.  In ASCII "AADW" ;-)
#define MAGIC_NUMBER (uint32_t)0x41414457


//=========================================================================================================
// Constructor() - Saves wear-leveling setup information and initializes our internal data-descriptor
//=========================================================================================================
CEEPROM_Base::CEEPROM_Base(uint16_t slot_count, uint16_t slot_size)
{
    // Save our wear-leveling parameters
    m_slot_count = slot_count;
    m_slot_size = slot_size;

    // By default we perform "dirty checking" on the data structure prior to writing it to physical EEPROM
    m_is_dirty_checking = true;

    // The EEPROM data structure in RAM is not yet dirty
    m_is_dirty = false;

    // Just in case someone calls "write()" without a "read()" first, clear the header
    memset(&m_header, 0, sizeof m_header);

    // Just for good measure, clear the error status
    m_error = error_t::OK;
}
//=========================================================================================================



//=========================================================================================================
// read() - Reads the physical EEPROM into our structure
//=========================================================================================================
bool CEEPROM_Base::read()
{
    // Presume for a moment that this routine is going to succeed
    m_error = error_t::OK;

    // Fetch information about our data structure
    const dd_t& data = data_descriptor();

    // Ensure that the wear-leveling slots are large enough to hold our data structure!!
    if (m_slot_count > 1 && m_slot_size < data.length + sizeof m_header)
    {
        m_error = error_t::BUG;
        return false;
    }

    // Our main data structure always defaults to all zeros.  This will ensure that if our
    // structure in RAM is longer than the structure in EEPROM, the new fields in RAM will
    // be initialized to zero
    memset(data.ptr, 0, data.length);

    // Fetch the header for the most recent edition of our structure that exists in EEPROM
    if (!find_most_recent_edition(&m_header))
    {
        m_error = error_t::IO;
    }

    // If a valid edition header was found, read in the main data
    if (m_header.magic == MAGIC_NUMBER)
    {
        // We want to read in every byte of the data structure in EEPROM
        uint16_t read_length = m_header.data_len;

        // Make sure it doesn't overflow our data structure in RAM!
        if (read_length > data.length) read_length = data.length;
 
        // Find the EEPROM address of this edition's header
        uint16_t address = edition_to_address(m_header.edition);
        
        // Read the data structure from EEPROM into RAM
        if (!read_physical_block(data.ptr, address + sizeof m_header, read_length))
        {
            m_error = error_t::IO;
        }

        // If there are no errors so far, check to see if the data we read was corrupted
        if (m_error == error_t::OK && m_header.crc != compute_crc(m_header.data_len))
        {
             m_error = error_t::CRC;
        }
    }
   
    // If we couldn't find a valid edition in EEPROM, clear the header and data fields to zeros
    else
    {
        memset(&m_header, 0, sizeof m_header);
        memset(data.ptr, 0, data.length);
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
// Returns: true on success, false if an I/O error occurs
//=========================================================================================================
bool CEEPROM_Base::find_most_recent_edition(header_t* p_result)
{
    header_t  header;

    // If we don't find any edition of our data structure in EEPROM, we'll return a cleared header
    memset(p_result, 0, sizeof header_t);

    // Loop through every possible slot
    for (int slot = 0; slot < m_slot_count; ++slot)
    {
        // Find the EEPROM address of this slot
        uint16_t address = slot_to_address(slot);

        // Fetch this slot's header from EEPROM
        if (!read_physical_block(&header, address, sizeof header)) return false;

        // If this is a valid header and is the the most recent edition so far, report it
        if (header.magic == MAGIC_NUMBER && header.edition > p_result->edition) *p_result = header;
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
    // Presume for a moment that this routine is going to succeed
    m_error = error_t::OK;

    // Fetch information about our data structure
    const dd_t& data = data_descriptor();

    // Ensure that the wear-leveling slots are large enough to hold our data structure!!
    if (m_slot_count > 1 && m_slot_size < data.length + sizeof m_header)
    {
        m_error = error_t::BUG;
        return false;
    }

    // If we're not forcing the write, and the data isn't "dirty", don't commit it to EEPROM
    if (!force_write && !is_dirty()) return true;

    // Fill in all of the header fields
    m_header.magic    = MAGIC_NUMBER;
    m_header.data_len = data.length;
    m_header.format   = data.format;
    
    // We are about to write a new edition of the data to EEPROM
    ++m_header.edition;

    // Fill in the CRC of the header and data 
    m_header.crc = compute_crc(data.length);

    // Find the EEPROM address where this edition should be written
    uint16_t address = edition_to_address(m_header.edition);

    // Write the header structure to EEPROM
    if (!write_physical_block(&m_header, address, sizeof m_header))
    {
        m_error = error_t::IO;
    }

    // Write the data structure to EEPROM
    if (!write_physical_block(data.ptr, address + sizeof m_header, data.length))
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
    // Presume for a moment that this routine is going to succeed
    m_error = error_t::OK;

    // Fetch information about our data structure
    const dd_t& data = data_descriptor();

    // Ensure that the wear-leveling slots are large enough to hold our data structure!!
    if (m_slot_count > 1 && m_slot_size < data.length + sizeof m_header)
    {
        m_error = error_t::BUG;
        return false;
    }

    // Fetch the header for the most recent edition of our structure that exists in EEPROM
    if (!find_most_recent_edition(&m_header))
    {
        m_error = error_t::IO;
        return false;
    }

    // If we found an edition that can be rolled back...
    if (m_header.magic == MAGIC_NUMBER)
    {
        // Find the EEPROM address of this edition
        uint16_t address = edition_to_address(m_header.edition);

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

    // Fetch information about our data structure
    const dd_t& data = data_descriptor();

    // Ensure that the wear-leveling slots are large enough to hold our data structure!!
    if (m_slot_count > 1 && m_slot_size < data.length + sizeof m_header)
    {
        m_error = error_t::BUG;
        return false;
    }

    // Destroy our header in RAM
    memset(&m_header, 0xFF, sizeof m_header);

    // Loop through every slot in EEPROM...
    for (int slot = 0; slot < m_slot_count; ++slot)
    {
        // Compute the EEPROM address of this slot
        uint16_t address = slot_to_address(slot);

        // And destroy the header in this slot
        if (!write_physical_block(&m_header, address, sizeof m_header))
        {
            m_error = error_t::IO;
        }
    }


    // EEPROM has been destroyed.  Set up the appropriate structures in RAM
    memset(&m_header, 0, sizeof m_header);
    memset(data.ptr, 0, data.length);
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
    // Fetch information about our data structure
    const dd_t& data = data_descriptor();

    // Save the existing CRC so we can restore it
    uint32_t old_crc = m_header.crc;

    // We don't want the CRC field to affect the CRC calculation
    m_header.crc = 0;

    // Compute the CRC
    uint32_t partial_crc = crc32(&m_header, sizeof m_header);
    uint32_t new_crc = crc32(data.ptr, data.length, partial_crc);

    // Restore the previous CRC
    m_header.crc = old_crc;

    // And hand the resulting CRC to the caller
    return new_crc;
}
//=========================================================================================================




//=========================================================================================================
// edition_to_address() - Converts an edition number to an EEPROM address
//=========================================================================================================
uint16_t CEEPROM_Base::edition_to_address(uint32_t edition)
{
    // Convert the edition number into a slot number
    int slot = (edition - 1) % m_slot_count;

    // And return the EEPROM address of this slot
    return slot_to_address(slot);
}
//=========================================================================================================




//=========================================================================================================
// slot_to_address() - Converts a wear_leveling slot number to an EEPROM address
//=========================================================================================================
uint16_t CEEPROM_Base::slot_to_address(int slot)
{
    // If we're not doing wear leveling, everything always goes into slot 0
    if (m_slot_count == 1) return 0;

    // Return the EEPROM address of this slot
    return slot * m_slot_size;
}
//=========================================================================================================


//=========================================================================================================
// mark_data_as_clean() - Marks the data structure in RAM as "clean" (i.e, the data in EEPROM exactly
//                        matches the data in RAM)
//=========================================================================================================
void CEEPROM_Base::mark_data_as_clean()
{
    // Fetch information about our data structure
    const dd_t& data = data_descriptor();

    // If the derived class wants to do automatic dirty checking, save a clean copy of the data
    if (data.clean_copy) memcpy(data.clean_copy, data.ptr, data.length);

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
    // Fetch information about our data structure
    const dd_t& data = data_descriptor();

    // If we're not doing "dirty checking", then we always assume the data is dirty
    if (!m_is_dirty_checking) return true;
    
    // If we're doing automatic "dirty checking", see if the data differs from the clean copy
    if (data.clean_copy && memcmp(data.ptr, data.clean_copy, data.length) != 0) return true;

    // Last but not least, find out if the derived class has told us the data is dirty
    return m_is_dirty;
}
//=========================================================================================================
