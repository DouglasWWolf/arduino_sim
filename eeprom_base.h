//=========================================================================================================
// CEEPROM_Base - Base class for a full featured EEPROM manager
// 
// Key features:
//     Portable and fully hardware independent
//     Data corruption detection via 32-bit CRC
//     Optional wear-leveling
//     Optional automatic dirty-checking prior to writing to physical EEPROM
//     The ability to "roll-back" a write, as though the write never happened
//     Seamless management of new EEPROM formats
//     Manages storage devices of up to 64K
//=========================================================================================================
#ifndef _EEPROM_BASE_H_
#define _EEPROM_BASE_H_
#include <stdint.h>

class CEEPROM_Base
{
public:

    // These are the types of errors that can occur
    enum class error_t : char { OK, IO, CRC, BUG };

    // Constructor.  Optionally specify number of wear-leveling slots and their size
    // *** Derived constructors MUST FILL IN THE m_data DATA DESCRIPTOR *****
    CEEPROM_Base(uint16_t slot_count = 1, uint16_t slot_size = 0);

    // Call this to disable (or re-enable) dirty checking.  It's enabled by default
    void    enable_dirty_checking(bool flag) { m_is_dirty_checking = flag; }

    // Read the physical EEPROM into the data structure in RAM
    bool    read();

    // Write the data structure in RAM to physical EEPROM
    bool    write(bool force = false);

    // Restore the EEPROM and data structure to where it was before the most recent "write()"
    bool    roll_back();

    // Wipes out the data structures in EEPROM
    bool    destroy();

    // Fetch the error code after a failed read, write, roll_back, or destroy operation
    error_t get_error() { return m_error; }


protected:

        
    // Pure virtual function to initialize new fields when the dataformat changes
    virtual void initialize_new_fields() = 0;

    // Pure virtual functions to perform physical I/O to the EEPROM
    virtual bool write_physical_block(void* src,  uint16_t address, uint16_t length) = 0;
    virtual bool read_physical_block (void* dest, uint16_t address, uint16_t length) = 0;

    //-----------------------------------------------------------------------
    // The order of these fields must not be disturbed!  
    //
    // A const header_t MUST BE THE VERY FIRST FIELD IN YOUR DATA STRUCTURE
    //-----------------------------------------------------------------------
    struct header_t
    {
        uint32_t    crc;
        uint32_t    edition;
        uint32_t    magic;
        uint16_t    data_len;
        uint16_t    format;
    };
    //-----------------------------------------------------------------------

    // Data descriptor - describes the user's data structure
    struct { void* ptr; uint16_t length; uint16_t format; void* clean_copy; } m_data;

    // This is the error code set by one of our public API calls
    error_t     m_error;

    // Number and size of storage "slots" for wear-leveling
    uint16_t    m_slot_count;
    uint16_t    m_slot_size;

    // If this is true, we're performing "dirty checking" of the data prior to writing 
    bool        m_is_dirty_checking;

    // If we are "dirty checking", derived classes can set this flag to indicate the data is dirty
    bool        m_is_dirty;

    // Returns true if the derived data structure won't fit into a wear-leveling slot
    bool        bug_check();

    // Computes the CRC of the header + data
    uint32_t    compute_crc(size_t data_length);

    // Returns the header of the most recent edition of our data structure found in EEPROM
    bool        find_most_recent_edition(header_t*, int*);

    // Returns the least recently used slot
    bool        find_least_recent_slot(int*);


    // Converts a 0 thru N slot number into an EEPROM address
    uint16_t    slot_to_header_address(int slot);
    uint16_t    slot_to_data_address(int slot);

    // Converts an edition number to an EEPROM address
    uint16_t    edition_to_address(uint32_t edition);

    // Reports whether the data-structure in RAM is "dirty" (i.e., requires flushing to EEPROM)
    bool        is_dirty();

    // Mark the data in the RAM structure as "clean"
    void        mark_data_as_clean();
};


#endif