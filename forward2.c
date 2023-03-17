/**
 *   In nic promisc mode, we recv pkt and forward to application layer.
 *   use netfilter.
 *
 *
 *    author: xxxx
 *    date: 2014-06-09
 *
 */
#include <linux/kernel.h>
#include <linux/module.h>
#include <asm/atomic.h>
#include <linux/ip.h>
#include <linux/version.h>
#include <linux/skbuff.h>
#include <linux/netfilter.h>
#include <linux/netfilter_ipv4.h>
#include <linux/moduleparam.h>
#include <linux/netfilter_ipv4/ip_tables.h>
#include <linux/if_arp.h>
#include <net/ip.h>
#include <linux/sched.h>
#include <asm/current.h>
#include <linux/nsproxy.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("db");
MODULE_DESCRIPTION("port-image-forward");

static struct nf_hook_ops nfho;

int forward_func(unsigned int hooknum, struct sk_buff *skb,
                const struct net_device *in,
                const struct net_device *out,
                int (*okfn)(struct sk_buff *))
{
    struct iphdr *iph;
    struct udphdr *udph;
    struct ethhdr *eh;
    struct net_device *dev;
    unsigned char *dp;

    if(!skb) 
        return NF_ACCEPT;
    
    if(!(eh = eth_hdr(skb)))
        return NF_ACCEPT;

    if(!(iph = ip_hdr(skb)))
        return NF_ACCEPT;

    if(skb->dev->type != ARPHRD_ETHER)
        return NF_ACCEPT;

    if(iph->protocol != IPPROTO_UDP)
        return NF_ACCEPT;
    
    udph = (struct udphdr *)(skb->data + (iph->ihl*4));
    if(udph->dest == ntohs(53)){
   		skb->pkt_type = PACKET_HOST;
    }

    return NF_ACCEPT;
}

int init_module()
{
    nfho.hook     = (nf_hookfn *)forward_func;
    nfho.hooknum  = NF_INET_PRE_ROUTING;
    nfho.pf       = PF_INET;
    nfho.priority = NF_IP_PRI_FIRST;
    nfho.owner    = THIS_MODULE;

    nf_register_hook(&nfho);

    return 0;
}

void cleanup_module()
{
    nf_unregister_hook(&nfho);
}
