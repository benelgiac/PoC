#ifndef INCLUDE_ETH_NUMBERS_
#define INCLUDE_ETH_NUMBERS_

/* 
* File:   eth_numbers.h
* Author: giacomo_benelli
*
* Created on 22 settembre 2009, 17.04
*
* Eth types and Ip protocol as taken from IANA documentation.
*/


/* Ethernet type field, on 16 bits */

/* NOTICE: the "type" field can assume only values higher than
 *  0x05DC. Smaller values must be interpreted as field "lenght"
 *  of IEEE802.3 "Ethernet II" 
 */
 
/* Directly from Wikipedia:
    Versions 1.0 and 2.0 of the Digital/Intel/Xerox (DIX) Ethernet specification 
have a 16-bit sub-protocol label field called the EtherType. The new IEEE 802.3 
Ethernet specification replaced that with a 16-bit length field, with the MAC 
header followed by an IEEE 802.2 logical link control (LLC) header. The maximum 
length of a frame was 1518 bytes for untagged (1522 for 802.1p or 802.1q tagged)
classical Ethernet v2 and IEEE802.3 frames. The two formats were eventually 
unified by the convention that values of that field between 64 and 1522 
indicated the use of the new 802.3 Ethernet format with a length field, while 
values of 1536 decimal (0600 hexadecimal) and greater indicated the use of the
original DIX or Ethernet II frame format with an EtherType sub-protocol 
identifier.[9] This convention allows software to determine whether a frame is
an Ethernet II frame or an IEEE 802.3 frame, allowing the coexistence of both 
standards on the same physical medium. See also Jumbo Frames.
    By examining the 802.2 LLC header, it is possible to determine whether it is 
followed by a SNAP (subnetwork access protocol) header. Some protocols, 
particularly those designed for the OSI networking stack, operate directly on 
top of 802.2 LLC, which provides both datagram and connection-oriented network 
services. The LLC header includes two additional eight-bit address fields, 
called service access points or SAPs in OSI terminology; when both source and 
destination SAP are set to the value 0xAA, the SNAP service is requested. The 
SNAP header allows EtherType values to be used with all IEEE 802 protocols, as 
well as supporting private protocol ID spaces. In IEEE 802.3x-1997, the IEEE 
Ethernet standard was changed to explicitly allow the use of the 16-bit field 
after the MAC addresses to be used as a length field or a type field.
*/
#define ETH_MAX_LEN              0x05DC

#define ETH_TYPE_IPv4            0x0800
#define ETH_TYPE_ARP             0x0806
#define ETH_TYPE_VLANTAG         0x8100 /*IEEE 802.1Q VLAN-tagged frames*/
#define ETH_TYPE_IPv6            0x86DD
//CECCO
#define ETHERTYPE_MPLS             0x8847
#define ETHERTYPE_MPLS_MULTI     0x8848

/* IP Protocol field, 8 bits*/
#define IPv4_PROTO_ICMP            0x01
#define IPv4_PROTO_IGMP            0x02
#define IPv4_PROTO_TCP             0x06
#define IPv4_PROTO_UDP             0x11
#define IPv4_PROTO_SCTP            0x84

#define MAC_ADDRESS_LEN          (6)
#define ETH_TYPE_LEN             (2)
#define MPLS_32_BITS             (4)

#define GETH_ETH_HEADER_LEN 14
#define GETH_ETH_VLANTAG_HEADER_LEN 18
#define ETH_TYPE_OFFSET          12
#define VLANTAG_ETH_OFFSET          12
#define VLANTAG_RC_FIELD_LEN        2
#define VLANTAG_TPID_FIELD_LEN        2
#define VLANTAG_TCI_FIELD_LEN        2

#define MAX_POSSIBLE_ETH_TYPES      0xFFFF
#define MAX_POSSIBLE_IP_VERSION      0x0F
#define MAX_POSSIBLE_IP_PROTOCOL      0xFF

typedef struct {
    UInt8 user_priority:3;
    UInt8 cfi:1;
    UInt16 vid:12;
} TCI_802_1Q;

typedef struct {
    UInt8 rt:3;
    UInt8 len:5;
    UInt8 d:1;
    UInt8 lf:6;
    UInt8 ncfi:1;
} ERIF_RC_FIELD;


#define IPv4_PROTO_SCTP     0x84
#define IPv4_PROTO_TCP      0x06
#define IPv4_PROTO_UDP      0x11

#endif