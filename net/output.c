#include "ns.h"

extern union Nsipc nsipcbuf;

void
output(envid_t ns_envid)
{
	binaryname = "ns_output";

	// LAB 6: Your code here:
	// 	- read a packet from the network server
	//	- send the packet to the device driver
	uint32_t whom;
	int perm;
	int32_t req;

	while (1) {
		req = ipc_recv((envid_t *)&whom, &nsipcbuf,  &perm);     //���պ���������̷���������
		if (req != NSREQ_OUTPUT) {
			cprintf("not a nsreq output\n");
			continue;
		}

    	struct jif_pkt *pkt = &(nsipcbuf.pkt);
    	while (sys_dl_transmit(pkt->jp_data, pkt->jp_len) < 0) {        //ͨ��ϵͳ���÷������ݰ�
       		sys_yield();
    	}	
	}
}
