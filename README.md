# cs417exercise1
repo for Chanah and Junjie
Goal: 
We will be programming a UDP/IP protocol that will both act reliable and efficient even with high packet loss. A variation of selective repeat ARQ protocol will be used to implement this system. 

Programming language:
C language

Data structure:
Types of Packets
Sender Initialization Packet (0): “I’m going to send you this file, are you ready?” 
file name
file type
file size (# of packets)
Receiver Response to Sender Initialization (1): “I’m ready or I’m not ready to receive”
approval or disapproval (boolean)
Sender Packet (2): “Actual file transfer starts”
file data
sequence number
Receiver Feedback Packet (3): “I have got this data most recently… but I’m missing this”
all negative acknowledgments 
accumulated acknowledgment

Packet 
packet type (0 - 3)
content of packet

Receiver Data Structure 
initialize the array of pointers, same number as packet numbers, which will point to its data received from the sender packet
unserialization function which will classify information from the packet
serialization function to build a packet
Sender Data Structure 
unserialization function which will classify information from the packet
serialization function to build a packet

Network Communication Protocol:
Algorithm
A variation of selective repeat ARQ protocol will be utilized
instead of receiver instantly giving feedback about a packet (nack or ack) we wait for a certain threshold (most likely with # of packets received)
we send a cumulative ack and multiple nacks at once 
Sender
Responds to request
Send packets in the window
Slide window according to the ack

Receiver
timer for last received packet, if timeout, then resend the feedback packet
slide window everytime it receives
keep track of nacks and largest ack under nack 
sends feedback every certain number of packets 

