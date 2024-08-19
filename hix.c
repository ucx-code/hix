/*
hix v.1.0 - Hypercall Invoker on Xen (hix)

Description:
    - This C program was developed to support a conducted study by a CISUC team on hypervisors and their robustness by invoking 
    hypercalls with border/Null parameters. This code is destined for x86 systems with the Xen hypervisor installed. 
    For further information, please visit our github repository: https://github.com/ucx-code/hix

Compilation:
    - To compile, simply run the "make" command on the c file folder;
    - In case of an error/warning, please run the following command:

        gcc  -O2 -fomit-frame-pointer -marm -fno-strict-aliasing -std=gnu99 -Wall -Wstrict-prototypes 
        -Wdeclaration-after-statement -Wno-unused-but-set-variable -Wno-unused-local-typedefs   -D__XEN_TOOLS__ 
        -MMD -MF .hix.o.d -D_FILE_OFFSET_BITS=64 -D_LARGEFILE_SOURCE -D_LARGEFILE64_SOURCE   
        -Werror -I/root/xen/tools/xcutils/../../tools/libxc/include -I/root/xen/tools/xcutils/../../tools/include 
        -I/root/xen/tools/xcutils/../../tools/libxc -I/root/xen/tools/xcutils/../../tools/include -c -o hix.o hix.c

Execution:
    - ./hix hypercall_id hypercall_specific_params


Developer Note: Any identified bugs are to be reported. This would greatly contribute to the gradual improvement of our tool!
*/


#include <err.h>
#include <stdint.h>
#include <stdio.h>
#include <time.h>

#include <xenctrl.h>
#include <xenguest.h>
#include "../libs/ctrl/xc_private.h"


int main(int argc, char* argv[]){
    int test = -55;
    char * arr = malloc(1000);
    XEN_GUEST_HANDLE_64(char) buffer;
    struct xen_platform_op* op = malloc(sizeof(*op));
    struct xen_sysctl* s = malloc(sizeof(*s));
    struct xenpf_settime64 timer;
    xc_interface *xch;
    xc_hypercall_buffer_t *hb;
    int hyp = atoi(argv[1]); // hypercall identification number
    xch = xc_interface_open(0,0,0);

    if ( !xch ){
        errx(1,"xcutils: hix.c: failed to open control interface");
    }

    switch (hyp)
    {

    case 12: // memory_op
        if(strcmp(argv[3],"NULL")==0){
            test = do_memory_op(xch, atoi(argv[2]), NULL, 0);
        }
        else{
            test = do_memory_op(xch, atoi(argv[2]), (void *) arr, atoi(argv[3]));
        }
        break;

    case 17: // xen_version
        if(argc-2 == 2){
            if(strcmp(argv[3],"NULL")==0){
                test = xc_version(xch, atoi(argv[2]), NULL);
            }
            else{
                uint64_t tmp2 = atoi(argv[3]);
                test = xc_version(xch, atoi(argv[2]), (void *) tmp2);
            }
        }else{printf("Invalid number of args\n");}
        break;

    case 18: // console_io
	    if(strcmp(argv[3],"NULL")==0){
            test = xencall3(xch->xcall, __HYPERVISOR_console_io, atoi(argv[2]), atoi(argv[3]), (uint64_t) NULL);
        }
        else{
            test = xencall3(xch->xcall, __HYPERVISOR_console_io, atoi(argv[2]), atoi(argv[3]), (uint64_t) &buffer);
        }
        break;

    case 20: // grant_table_op
	    if(strcmp(argv[3],"NULL")==0){
            test = xencall3(xch->xcall, __HYPERVISOR_grant_table_op, atoi(argv[2]), (uint64_t) NULL, atoi(argv[3]));
        }
        else{
            test = xencall3(xch->xcall, __HYPERVISOR_grant_table_op, atoi(argv[2]), (uint64_t) &buffer, atoi(argv[3]));
        }
        break;

    case 21: // vm_assist
        test = xencall2(xch->xcall, __HYPERVISOR_vm_assist, atoi(argv[2]), atoi(argv[3]));
        break;

    case 24: // vcpu_op
	    if(strcmp(argv[3],"NULL")==0){
            test = xencall3(xch->xcall, __HYPERVISOR_vcpu_op, atoi(argv[2]), atoi(argv[3]), (uint64_t) NULL);
        }
        else{
        	test = xencall3(xch->xcall, __HYPERVISOR_vcpu_op, atoi(argv[2]), atoi(argv[3]), (uint64_t) &buffer);
        }
        break;

    case 29: // sched_op
	    if(strcmp(argv[2],"NULL")==0){
            test = xencall2(xch->xcall, __HYPERVISOR_sched_op, atoi(argv[2]), (uint64_t) NULL);
        }
        else{
            buffer.p = NULL;
        	test = xencall2(xch->xcall, __HYPERVISOR_sched_op, atoi(argv[2]), (uint64_t) &buffer);
        }
        break;

    case 33: // physdev_op
        if(arr==NULL){printf("malloc unsuccessful\n");break;}
        if(argc-2 == 2){
		    test = do_physdev_op(xch, atoi(argv[2]), (void *) arr, atoi(argv[3]));
	    }else{printf("Invalid number of args\n");}
        break;

    case 35: // sysctl_op
	    buffer.p = NULL;

        s->cmd = atoi(argv[2]); // 1
        s->interface_version = XEN_SYSCTL_INTERFACE_VERSION;
  
        srand(atoi(argv[3]));

        for(int i = 0; i< 128; i++){s->u.pad[i] = rand()*255;}

        test = xc_sysctl(xch, s);
        break;

    default:
        printf("Invalid Hypercall: %d\n", hyp);
        break;
    }

    xc_interface_close(xch);
	return test;

}
