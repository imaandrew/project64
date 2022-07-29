#pragma once
#include "MemoryHandler.h"
#include <Project64-core\Settings\GameSettings.h>

class CMipsMemoryVM;
class CHomeboyFIFO;

class HomeboyHandler:
    public MemoryHandler
{
public:
    HomeboyHandler(CMipsMemoryVM & MMU, CHomeboyFIFO & FIFO);
    ~HomeboyHandler();

    bool Read32(uint32_t Address, uint32_t & Value);
    bool Write32(uint32_t Address, uint32_t Value, uint32_t Mask);

private:
    HomeboyHandler();
    HomeboyHandler(const HomeboyHandler &);
    HomeboyHandler & operator=(const HomeboyHandler &);

    static uint32_t swap32by8(uint32_t word);

    CMipsMemoryVM & m_MMU;
    CHomeboyFIFO & m_FIFO;
    uint32_t m_hb_dram_addr;
    uint32_t m_hb_n_blocks;
    uint32_t m_hb_status;
    FILE * m_hb_file;
    
    uint32_t m_hb_fifo_dram_addr;
    uint32_t m_hb_fifo_rd_len;
    uint32_t m_hb_fifo_wr_len;
};
