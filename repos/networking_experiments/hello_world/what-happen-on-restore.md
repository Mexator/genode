## What will happen when we rollback server?

If server haven't started communicating with client, or already finished - 
nothing will happen for client. Server will be transparently rolled back. 
(This is valid for CLOSED and LISTEN states).

If server rollbacks in SYN_RCWD state, it will respond with RST to ACK sent by 
client. Client will shutdown connection.

If server rollbacks in ESTABLISHED state, and client sends payload to server,
it will do one of two things:

* If server after rollback is in ESTABLISHED state, and before rollback it did
not ACKed any packets that are not received after rollback, it just sends ACK
of last received packet and client retransmits all in-flight data.

* If server ACKed seq_no < than was ACKed before, client responds with RST, as
ACKed seq_no is not within window size. (Not sure about this, need to test)

If server rolls back from any other state, client will respond with RST.

## What will happen when we rollback client?

If server haven't started communicating with client, or already finished - 
nothing will happen for client. Client will be transparently rolled back. 
(This is valid for CLOSED and SYN_SENT states).

If client rollbacks in SYN_SENT state back to CLOSED state, it will receive 
unexpected SYN+ACK from server. Client responds with RST and server closes 
connection. Then client sends initial SYN and handshake process restarts from 
the very beginning.

If client rollbacks in ESTABLISHED state, the server will ACK packets that are 
not yet sent by client. In this case client would send RST, as it not expects
this packets to be acknowledged.

If client rolls back from any other (closing) state, server will respond with 
RST, as it expects connection to be closed.
