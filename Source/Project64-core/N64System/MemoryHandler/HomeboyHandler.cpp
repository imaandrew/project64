#include "stdafx.h"
#include "HomeboyHandler.h"
#include <Project64-core\N64System\N64System.h>
#include <Project64-core\N64System\SystemGlobals.h>

HomeboyHandler::HomeboyHandler(CMipsMemoryVM & MMU, CHomeboyFIFO & FIFO) :
    m_MMU(MMU),
    m_FIFO(FIFO),
    m_hb_dram_addr(0),
    m_hb_n_blocks(0),
    m_hb_status(0),
    m_hb_file(nullptr),
    
    m_hb_fifo_dram_addr(0),
    m_hb_fifo_rd_len(0),
    m_hb_fifo_wr_len(0)
{
}

HomeboyHandler::~HomeboyHandler()
{
    if (m_hb_file)
        fclose(m_hb_file);
}

bool HomeboyHandler::Read32(uint32_t Address, uint32_t & Value)
{
    switch ((Address & 0x0000FFFF))
    {
    case 0x0000:
    {
        Value = 0x1234;
        break;
    }
    case 0x0014: Value = m_hb_status; break;
    case 0x002C:
    {
        uint32_t status = 0;

        SOCKET hb_socket = m_FIFO.get_client();

        if (hb_socket != INVALID_SOCKET)
        {
            fd_set readfds;
            fd_set writefds;
            struct timeval tv;

            FD_ZERO(&readfds);
            FD_ZERO(&writefds);
            FD_SET(hb_socket, &readfds);
            FD_SET(hb_socket, &writefds);

            tv.tv_sec = 0;
            tv.tv_usec = 0;

            select(0, &readfds, &writefds, NULL, &tv);

            status |= (1 << 0);

            if (!FD_ISSET(hb_socket, &readfds))
                status |= (1 << 1);

            if (!FD_ISSET(hb_socket, &writefds))
                status |= (1 << 2);

            m_FIFO.put_client();
        }

        Value = status;
        break;
    }
    case 0x0030: Value = m_hb_fifo_dram_addr; break;
    case 0x0034: Value = m_hb_fifo_rd_len; break;
    case 0x0038: Value = m_hb_fifo_wr_len; break;
    }
    return true;
}

bool HomeboyHandler::Write32(uint32_t Address, uint32_t Value, uint32_t /*Mask*/)
{
    switch ((Address & 0x0000FFFF))
    {
    case 0x0004: m_hb_dram_addr = Value; break;
    case 0x0008:
    {
        uint32_t lba = Value;
        uint32_t size = m_hb_n_blocks * 512;
        m_hb_status &= ~(0b1111 << 5);
        if (!m_hb_file ||
            m_hb_dram_addr >= m_MMU.RdramSize() ||
            m_hb_dram_addr + size > m_MMU.RdramSize())
        {
            m_hb_status |= (1 << 5);
            return true;
        }
        uint32_t * src = (uint32_t *)(m_MMU.Rdram() + m_hb_dram_addr);
        uint32_t * buf = (uint32_t *)malloc(size);
        if (!buf)
        {
            m_hb_status |= (3 << 5);
            return true;
        }
        memcpy(buf, src, size);
        for (uint32_t i = 0; i < size / 4; ++i)
            buf[i] = swap32by8(buf[i]);
        if (fseek(m_hb_file, lba * 512, SEEK_SET) != 0 || fwrite(buf, size, 1, m_hb_file) != 1)
        {
            m_hb_status |= (1 << 5);
            return true;
        }
        free(buf);
        break;
    }
    case 0x000C:
    {
        uint32_t lba = Value;
        uint32_t size = m_hb_n_blocks * 512;
        m_hb_status &= ~(0b1111 << 5);
        if (!m_hb_file ||
            m_hb_dram_addr >= m_MMU.RdramSize() ||
            m_hb_dram_addr + size > m_MMU.RdramSize())
        {
            m_hb_status |= (1 << 5);
            return true;
        }
        uint32_t * dst = (uint32_t *)(m_MMU.Rdram() + m_hb_dram_addr);
        if (fseek(m_hb_file, lba * 512, SEEK_SET) != 0 || fread(dst, size, 1, m_hb_file) != 1)
        {
            m_hb_status |= (1 << 5);
            return true;
        }
        for (uint32_t i = 0; i < size / 4; ++i)
            dst[i] = swap32by8(dst[i]);
        break;
    }
    case 0x0010: m_hb_n_blocks = Value; break;
    case 0x0014:
    {
        if (Value & (0b1 << 4))
        {
            m_hb_status &= ~((0b1 << 0) | (0b1 << 2) | (0b1 << 3) | (0b1111 << 5));
            if (m_hb_file)
                fclose(m_hb_file);
            if (!CGameSettings::bEnableSDCard())
                return true;
            m_hb_file = fopen(CGameSettings::sSDCardPath().c_str(), "r+b");
            if (!m_hb_file)
            {
                m_hb_status |= (1 << 5);
                return true;
            }
            m_hb_status |= (1 << 0) | (1 << 2) | (1 << 3);
        }
        break;
    }
    case 0x0030: m_hb_fifo_dram_addr = Value; break;
    case 0x0034:
    {
        uint32_t size = Value;
        m_hb_fifo_rd_len = size;

        if (m_hb_fifo_dram_addr >= m_MMU.RdramSize() ||
            m_hb_fifo_dram_addr + size > m_MMU.RdramSize())
        {
            return true;
        }

        SOCKET hb_socket = m_FIFO.get_client();
        if (hb_socket == INVALID_SOCKET)
        {
            return true;
        }

        uint8_t * dram = m_MMU.Rdram();
        uint8_t * buf = (uint8_t *) malloc(size);

        if (buf)
        {
            int recvd = recv(hb_socket, (char *) buf, size, 0);

            if (recvd == INVALID_SOCKET)
            {
                recvd = 0;
            }

            for (int i = 0; i < recvd; i++)
            {
                dram[(m_hb_fifo_dram_addr + i) ^ 3] = buf[i];
            }

            m_hb_fifo_dram_addr += recvd;
            m_hb_fifo_rd_len -= recvd;

            free(buf);
        }

        m_FIFO.put_client();

        break;
    }
    case 0x0038:
    {
        uint32_t size = Value;
        m_hb_fifo_wr_len = size;

        if (m_hb_fifo_dram_addr >= m_MMU.RdramSize() ||
            m_hb_fifo_dram_addr + size > m_MMU.RdramSize())
        {
            return true;
        }

        SOCKET hb_socket = m_FIFO.get_client();
        if (hb_socket == INVALID_SOCKET)
        {
            return true;
        }

        uint8_t * dram = m_MMU.Rdram();
        uint8_t * buf = (uint8_t *) malloc(size);

        if (buf)
        {
            int sent;

            for (uint32_t i = 0; i < size; i++)
            {
                buf[i] = dram[(m_hb_fifo_dram_addr + i) ^ 3];
            }

            sent = send(hb_socket, (char *) buf, size, 0);

            if (sent == INVALID_SOCKET)
            {
                sent = 0;
            }

            m_hb_fifo_dram_addr += sent;
            m_hb_fifo_wr_len -= sent;

            free(buf);
        }

        m_FIFO.put_client();

        break;
    }
    }
    return true;
}

uint32_t HomeboyHandler::swap32by8(uint32_t word)
{
    const uint32_t swapped =
#if defined(_MSC_VER)
        _byteswap_ulong(word)
#elif defined(__GNUC__)
        __builtin_bswap32(word)
#else
        (word & 0x000000FFul) << 24
        | (word & 0x0000FF00ul) << 8
        | (word & 0x00FF0000ul) >> 8
        | (word & 0xFF000000ul) >> 24
#endif
        ;
    return (swapped & 0xFFFFFFFFul);
}