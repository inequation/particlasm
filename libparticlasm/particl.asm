; particlasm main module
; Copyright (C) 2011, Leszek Godlewski <lg@inequation.org>

; declare the GOT symbol
extern _GLOBAL_OFFSET_TABLE_

global foo:function

foo:	push    ebp
        mov     ebp,esp
        push    ebx
        call    .get_GOT
.get_GOT:
        pop     ebx
        add     ebx,_GLOBAL_OFFSET_TABLE_+$$-.get_GOT wrt ..gotpc

        mov     eax, 0DEADC0DEh

        mov     ebx,[ebp-4]
        mov     esp,ebp
        pop     ebp
        ret

