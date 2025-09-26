#define ISP_XTL_MASK    (isp + 3 * 7)
#define ISP_XTL_PORT    (ISP_XTL_MASK + 1)
#define ISP_CK_MASK     (isp + 4 * 7)
#define ISP_CK_PORT     (ISP_CK_MASK + 1)
#define ISP_DO_MASK     (isp + 6 * 7)
#define ISP_DO_PORT     (ISP_DO_MASK + 1)
.extern isp

.global ispXtal

ispXtal:
    push r18
    mov	r18, r24
    lds r30, ISP_XTL_PORT
    ;lds r31, ISP_XTL_PORT+1
    ldi r31, 0
    ld	r24, Z          ; r24 = *port
    lds r25, ISP_XTL_MASK
    rjmp    ispXtalCheck
ispXtalStart:

    eor	r24, r25
    st	Z, r24
    eor	r24, r25
    st	Z, r24

    ;dec	r18
    subi    r18, 0x01
ispXtalCheck:
    and r18, r18
    brne    ispXtalStart
    pop r18
    ret


/*
.global ispTransmit_sw
ispTransmit_sw:
    push	r15
    push	r16
    push	r17
    push	r18
    push	r19
    push	r20
    push	r21
    mov	r15, r24
    ldi	r17, 0x00	; 0
    ldi	r16, 0x08	; 8

    lds	r26, 0x00BC
    ldi	r27, 0x00
    ld	r20, X
    lds	r21, 0x00BB

    lds	r28, 0x00AE
    ldi	r29, 0x00
    ld	r18, Y
    lds	r19, 0x00AD

ispTransmit_sw_start:

    subi	r16, 0x01	; 1
    sbrs	r15, 7
    rjmp	ispTransmit_sw_clr_mosi     	; 0xb8c <ispTransmit_sw+0x24>
    ;lds	r30, 0x00BC
    ;ldi	r31, 0x00
    ;ld	r20, X
    ;lds	r25, 0x00BB
    or	r20, r21
    rjmp	ispTransmit_sw_mosi     	; 0xb9e <ispTransmit_sw+0x36>

ispTransmit_sw_clr_mosi:

    ;lds	r30, 0x00BC
    ;ldi	r31, 0x00
    ;ld	r20, X
    ;lds	r24, 0x00BB
    com	r21
    and	r20, r21
    com	r21

ispTransmit_sw_mosi:

    st	X, r20
    add	r17, r17
    lds	r30, 0x00B7
    ldi r31, 0x00
    ;lds	r31, 0x00B8
    ld	r24, Z
    lds	r25, 0x00B4
    and	r24, r25
    breq	ispTransmit_sw_set_sck      	; 0xbb6 <ispTransmit_sw+0x4e>
    subi	r17, 0xFF	; 255

ispTransmit_sw_set_sck:

    ;lds	r30, 0x00AE
    ;lds	r31, 0x00AF
    ld	r18, Y
    ;lds	r25, 0x00AD
    or	r18, r19
    st	Y, r18
    rcall	ispDelay    	; 0xb42 <ispDelay>
    ;lds	r30, 0x00AE
    ;ldi	r31, 0x00
    ;ld	r18, Y
    ;lds	r24, 0x00AD
    ;com	r24
    ;and	r24, r25
    eor r18, r19
    st	Y, r18
    rcall	ispDelay    	; 0xb42 <ispDelay>
    and	r16, r16
    breq	ispTransmit_sw_end      	; 0xbe8 <ispTransmit_sw+0x80>
    add	r15, r15
    rjmp	ispTransmit_sw_start    	; 0xb74 <ispTransmit_sw+0xc>

ispTransmit_sw_end:

    mov	r24, r17
    pop	r21
    pop	r20
    pop	r19
    pop	r18
    pop	r17
    pop	r16
    pop	r15
    ret
*/
