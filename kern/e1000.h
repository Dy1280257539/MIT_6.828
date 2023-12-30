#ifndef JOS_KERN_E1000_H
#define JOS_KERN_E1000_H

#include <kern/pci.h>

#define REG_TDBAL    0x03800  /* TX Descriptor Base Address Low - RW */
#define REG_TDBAH    0x03804  /* TX Descriptor Base Address High - RW */
#define REG_TDLEN    0x03808  /* TX Descriptor Length - RW */
#define REG_TDH      0x03810  /* TX Descriptor Head - RW */
#define REG_TDT      0x03818  /* TX Descripotr Tail - RW */
#define REG_TCTL     0x00400  /* TX Control - RW */
#define REG_TIPG     0x00410  /* TX Inter-packet gap -RW */

#define TCTL_EN_BIT         (1 << 1)
#define TCTL_PSP_BIT        (1 << 3)
#define TCTL_CT_SHIFT       4
#define TCTL_COLD_SHIFT     12

#define TIPG_IPGT_SHIFT     0
#define TIPG_IPGR1_SHIFT    10
#define TIPG_IPGR2_SHIFT    20

#define TX_CMD_EOP_BIT     (1 << 0)
#define TX_CMD_IFCS_BIT    (1 << 1)
#define TX_CMD_RS_BIT      (1 << 3)
#define TX_STA_DD_BIT      (1 << 0)

struct e1000_tx_desc {
    uint64_t addr;
    uint16_t length;
    uint8_t cso;
    uint8_t cmd;
    uint8_t status;
    uint8_t css;
    uint16_t special;
};

#define E1000_NTXDESC 32
#define E1000_MAXPACK 1518

int pci_e1000_attach(struct pci_func *pcif);

int e1000_transmit(const char *buf, size_t len);



#define REG_RAL      0x05400
#define REG_RAH      0x05404
#define REG_RDBAL    0x02800  /* RX Descriptor Base Address Low - RW */
#define REG_RDBAH    0x02804  /* RX Descriptor Base Address High - RW */
#define REG_RDLEN    0x02808  /* RX Descriptor Length - RW */
#define REG_RDH      0x02810  /* RX Descriptor Head - RW */
#define REG_RDT      0x02818  /* RX Descriptor Tail - RW */
#define REG_RCTL     0x00100  /* RX Control - RW */

#define REG_MTA      0x05200  /* Multicast Table Array - RW Array */


#define RAH_AV_BIT              (1 << 31)

#define RCTL_EN_BIT             (1 << 1)
#define RCTL_LBM_SHIFT          6
#define RCTL_RDMTS_SHIFT        8
#define RCTL_BAM_BIT            (1 << 15)
#define RCTL_BSIZE_SHIFT        16
#define RCTL_SECRC_BIT          (1 << 26)

#define RX_STA_DD_BIT      (1 << 0)
#define RX_STA_EOP_BIT     (1 << 1)

struct e1000_rx_desc {
    uint64_t addr;
    uint16_t length;
    uint16_t checksum;
    uint8_t status;
    uint8_t errors;
    uint16_t special;
};


#endif  // SOL >= 6
