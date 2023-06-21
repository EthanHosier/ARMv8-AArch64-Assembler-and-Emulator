ldr w0, MAILBOX_BASE
ldr w1, READ_REGISTER
ldr w2, WRITE_REGISTER
ldr w3, STATUS_REGISTER
movz w30, ON_1, lsl #16
movk w30, ON_1
movz w28, OFF_1, lsl #16
movk w28, OFF_1

ldr w29, COUNTER


loop:
  ldr w4, [w0, w3] // w4 = status register
  orr w4, wzr, w4, lsr #31 // full flag
  cmp wzr, w4
  bne loop // wait for write queue to not be full
  
  
  mov w4, w30 // w4 = address of request to turn on LED
  orr w4, wzr, w4, lsl #4
  orr w4, w4, #8 // w4 = write message
  str w4, [w0, w2] // make request to turn on led
  wait_for_response: // wait for response to be in response queue
    ldr w4, [w0, w3] // w4 = status register
    orr w4, wzr, w4, lsr #30 // empty flag
    cmp wzr, w4
  bne wait_for_response // wait for read queue to not be empty
  ldr w4, [w0, w1] // reads response
  

  eor w10, w10, w10 // wait random time
	delay1:
	  add w10, w10, #1
	  cmp w10, w29
	bne delay1


  //check if room for request
  ldr w4, [w0, w3] // w4 = status register
  orr w4, wzr, w4, lsr #31 // full flag
  cmp wzr, w4
  

  mov w4, w28
  orr w4, wzr, w4, lsl #4
  orr w4, w4, #8 // w4 = write message
  str w4, [w0, w2] // make request to turn off led
  
  wait_for_response_2: // wait for response to be in response queue
    ldr w4, [w0, w3] // w4 = status register
    orr w4, wzr, w4, lsr #30 // empty flag
    cmp wzr, w4
  bne wait_for_response_2 // wait for read queue to not be empty
  ldr w4, [w0, w1] // reads response
  
  eor w10, w10, w10 // wait random time
	delay2:
	  add w10, w10, #1
	  cmp w10, w29
	bne delay2
b loop






COUNTER:
  .int 0x00800000

MAILBOX_BASE:
  .int 0x3f00b880

READ_REGISTER: // Read = MAILBOX_BASE + READ_REGISTER
  .int 0x00

WRITE_REGISTER:// Write = MAILBOX_BASE + WRITE_REGISTER
  .int 0x20

STATUS_REGISTER: // Status = MAILBOX_BASE + STATUS_REGISTER
  .int 0x38

CHANNEL:
  .int 8

ON_1:
  .int 0x00000020      // 32
  .int 0x00000000      // 0
ON_2:
  .int 0x00038041
  .int 0x00000008      // 8
ON_3:
  .int 0x00000000
  .int 0x00000082      // 130
ON_4:
  .int 0x00000001
  .int 0x00000000

OFF_1:
  .int 0x00000020      // 32
  .int 0x00000000      // 0
OFF_2:
  .int 0x00038041
  .int 0x00000008      // 8
OFF_3:
  .int 0x00000000
  .int 0x00000082      // 130
OFF_4:
  .int 0x00000000
  .int 0x00000000