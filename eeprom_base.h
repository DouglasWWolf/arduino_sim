//=========================================================================================================
// CEEPROM_Base - Base class for a full featured EEPROM manager
// 
// Key features:
//     Portable and fully hardware independent
//     Data corruption detection via 32-bit CRC
//     Optional wear-leveling
//     Optional caching of wear-leveling information for faster read/writes
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

    // Constructor.
    // *** Derived constructors MUST FILL IN THE m_data DATA DESCRIPTOR *****
    CEEPROM_Base();

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

    // Wear leveling configuration
    struct { uint16_t count; uint16_t size; uint32_t* cache; } m_wl;
    
    // This is the error code set by one of our public API calls
    error_t     m_error;

    // If this is true, we're performing "dirty checking" of the data prior to writing 
    bool        m_is_dirty_checking;

    // If we are "dirty checking", derived classes can set this flag to indicate the data is dirty
    bool        m_is_dirty;

    // Pointer to the wear-leveling cache if it exists
    uint32_t*   m_wl_cache;

    // Returns true if the derived data structure won't fit into a wear-leveling slot
    bool        bug_check();

    // Computes the CRC of the header + data
    uint32_t    compute_crc(size_t data_length);

    // Returns the header of the most recent edition of our data structure found in EEPROM
    bool        find_most_recent_edition(header_t*, uint16_t* p_address, int* p_slot = nullptr);

    // Returns the EEPROM address of the least recently used slot
    bool        find_least_recent_address(uint16_t* p_address, int* p_slot);

    // Converts a 0 thru N slot number into an EEPROM address
    uint16_t    slot_to_header_address(int slot);

    // Destroys the header in the specied EEPROM slot
    bool        destroy_slot(int slot);

    // Reports whether the data-structure in RAM is "dirty" (i.e., requires flushing to EEPROM)
    bool        is_dirty();

    // Mark the data in the RAM structure as "clean"
    void        mark_data_as_clean();

    // Builds the wear-leveling cache
    bool        build_wl_cache();

    // Reads a header from EEPROM into RAM
    bool        read_header(header_t* p_result, uint16_t address);

private:

    // This will be true if we have wear-leveling data cached
    bool        m_is_cached;
};


#endif