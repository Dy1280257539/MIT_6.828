#include <kern/e1000.h>
#include <kern/pmap.h>
#include <inc/string.h>

// LAB 6: Your driver code here

static volatile uint32_t *e1000;

__attribute__((__aligned__(sizeof(struct e1000_tx_desc))))
static volatile struct e1000_tx_desc tx_descs[E1000_NTXDESC];
static volatile uint8_t tx_buf[E1000_NTXDESC][E1000_MAXPACK];

static volatile uint32_t *reg_tdh, *reg_tdt;

static void 
tx_init() 
{
    // tx descs init
    for (int i = 0; i < E1000_NTXDESC; ++i) {
        tx_descs[i].status |= TX_STA_DD_BIT;
    }

    // setting TDBAL, TDLEN
    e1000[REG_TDBAH >> 2] = 0;
    e1000[REG_TDBAL >> 2] = (uint32_t)PADDR((void*)tx_descs);
    e1000[REG_TDLEN >> 2] = sizeof(tx_descs);

    // TDH = TDT = 0
    reg_tdh = &e1000[REG_TDH >> 2];
    reg_tdt = &e1000[REG_TDT >> 2];
    *reg_tdh = 0;
    *reg_tdt = 0;

    // TCTL setting
    e1000[REG_TCTL >> 2] = TCTL_EN_BIT 
        | TCTL_PSP_BIT 
        | (0x10 << TCTL_CT_SHIFT) 
        | (0x40 << TCTL_COLD_SHIFT)
        ;
    
    // IPG setting
    e1000[REG_TIPG >> 2] = 0
        | (10 << TIPG_IPGT_SHIFT) 
        | (8 << TIPG_IPGR1_SHIFT) 
        | (6 << TIPG_IPGR2_SHIFT)
        ;
}

int 
e1000_transmit(const char *buf, size_t len) 
{
    assert(len <= E1000_MAXPACK);

    uint32_t tmp_reg_tdt = *reg_tdt;

    // queue if full
    if (!(tx_descs[tmp_reg_tdt].status & TX_STA_DD_BIT))
        return -1;

    // copy memory
    memmove((void*)tx_buf[tmp_reg_tdt], buf, len);

    // setting desc
    tx_descs[tmp_reg_tdt].cmd = TX_CMD_RS_BIT 
        | TX_CMD_EOP_BIT 
        | TX_CMD_IFCS_BIT
        ;

    tx_descs[tmp_reg_tdt].status = 0;
    tx_descs[tmp_reg_tdt].addr = (uint64_t)PADDR((void*)tx_buf[tmp_reg_tdt]);
    tx_descs[tmp_reg_tdt].length = len;

    // update TDT
    *reg_tdt = (tmp_reg_tdt + 1) % E1000_NTXDESC;

    return 0;
}


#define E1000_NRXDESC 128

#define QEMU_MAC_ADDR  0x563412005452

__attribute__((__aligned__(sizeof(struct e1000_rx_desc))))
static volatile struct e1000_rx_desc rx_descs[E1000_NRXDESC];
static volatile uint8_t rx_buf[E1000_NRXDESC][E1000_MAXPACK];


static volatile uint32_t *reg_rdh, *reg_rdt;

static void 
rx_init() 
{
    // init rx descs
    for (int i = 0; i < E1000_NRXDESC; ++i) {
        rx_descs[i].addr = (uint32_t)PADDR((void*)rx_buf[i]);
        rx_descs[i].length = E1000_MAXPACK;
        rx_descs[i].status = 0;
    }

    // setting RAH:RAL
    e1000[REG_RAL >> 2] = QEMU_MAC_ADDR & 0xFFFFFFFF;
    e1000[REG_RAH >> 2] = QEMU_MAC_ADDR >> 32 | RAH_AV_BIT;

    // setting MTA: Multicast Table Array
    memset((void*)(e1000 + REG_MTA), 0, 128 * sizeof(uint32_t));

    // setting RDBAL, RDLEN
    e1000[REG_RDBAL >> 2] = (uint32_t)PADDR((void*)rx_descs);
    e1000[REG_RDLEN >> 2] = sizeof(rx_descs);

    // setting RDH, RHT
    reg_rdh = &e1000[REG_RDH >> 2];
    reg_rdt = &e1000[REG_RDT >> 2];
    *reg_rdh = 0;
    *reg_rdt = E1000_NRXDESC - 1;

    // setting RCTL
    e1000[REG_RCTL >> 2] = RCTL_EN_BIT
        | (0 << RCTL_LBM_SHIFT)
        | (3 << RCTL_RDMTS_SHIFT)
        | RCTL_BAM_BIT
        | (0 << RCTL_BSIZE_SHIFT)
        | RCTL_SECRC_BIT
        ;
}

int
e1000_receive(char *buf, size_t len)
{
    uint32_t tmp_reg_rdt = (*reg_rdt + 1) % E1000_NRXDESC;

    // queue is empty
    if (!(rx_descs[tmp_reg_rdt].status & RX_STA_DD_BIT))
        return -1;

    // one buffer, one packet
    assert(rx_descs[tmp_reg_rdt].status & RX_STA_EOP_BIT);
    assert(KADDR(rx_descs[tmp_reg_rdt].addr) == rx_buf[tmp_reg_rdt]);

    // memory copy
    len = MIN(len, rx_descs[tmp_reg_rdt].length);
    memmove(buf, KADDR(rx_descs[tmp_reg_rdt].addr), len);

    // set desc
    rx_descs[tmp_reg_rdt].status = 0;
    rx_descs[tmp_reg_rdt].addr = (uint32_t)PADDR((void*)rx_buf[tmp_reg_rdt]);
    rx_descs[tmp_reg_rdt].length = E1000_MAXPACK;

    // update register
    *reg_rdt = tmp_reg_rdt;

    return (int)len;
}

int
pci_e1000_attach(struct pci_func *pcif) 
{
    pci_func_enable(pcif);
    e1000 = mmio_map_region(pcif->reg_base[0], pcif->reg_size[0]);
    tx_init();
    rx_init();
    return 1;
}