#ifndef _EEPROM_MANAGER_H_
#define _EEPROM_MANAGER_H_
#include "eeprom_base.h"



class CEEPROM : public CEEPROM_Base
{
public:

/*
    //-------------------------------------------------------------------------------
    // These are all defined in the base class
    //-------------------------------------------------------------------------------

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

    // Fetch the error code after a read, save, or destroy operation
    error_t get_error() { return m_error; }

*/

public:

    // Constructor.  Optionally specify number of wear-leveling slots and their size
    CEEPROM(uint16_t slot_count = 1, uint16_t slot_size = 0);

    // **********************************************************************************
    // *** ABSOLUTELY ANY TIME THE DATA STRUCTURE CHANGES, THIS MUST BE INCREMENTED!! ***
    // **********************************************************************************
    enum {DATA_FORMAT = 1};


    // **********************************************************************************
    // ***    >>> The first field in this struct MUST be a const header_t!!! <<<      ***
    // ***                                                                            ***
    // *** You can add fields to this, but must never delete an existing field or     ***
    // *** change its size. If you add a field, increment DATA_FORMAT above!          ***
    // **********************************************************************************
    struct data_t
    {
        const header_t  header;
        uint8_t         run_mode;
    } data, clean;

protected:
    
    // **********************************************************************************
    // *** If you add a new field and need it initialized to something other than     ***
    // *** zero, add that initialization code to this routine                         ***
    // **********************************************************************************
    void initialize_new_fields();

    // Virtual functions to perform physical I/O to the EEPROM
    bool write_physical_block(void* src, uint16_t address, uint16_t length);
    bool read_physical_block(void* dest, uint16_t address, uint16_t length);

};


#endif