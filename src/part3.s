Peripheral_Base_Address: .int 0x3f000000 
Mailbox_Base: .int Peripheral_Base_Address + 0xb880 
Read_Register: .int Mailbox_Base + 0x00
Write_Register: .int Mailbox_Base + 0x20
Status_Register: .int Mailbox_Base + 0x38 
Channel: .int 0x8
On_Message: .int _________
Off_Message: .int _________

start:

wait_for_write_flag_zero:
    ldr w0, [xzr, Write_Register]
    tst wzr, w0, lsr #31
    bne wait_for_write_flag_zero

    movz x1, #0
    str _, Write_Register















    b start

