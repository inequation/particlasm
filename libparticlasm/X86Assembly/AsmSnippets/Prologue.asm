[list -]

; Particlasm emitter code
; Generated automatically, do not modify!

; =============================================================================
; BOILERPLATE STARTS HERE
; =============================================================================

; NASMX base include
;%include 'nasmx.inc'
{%s}

; some NASMX supplements
%ixdefine ptr_t				ptrdiff_t			; platform-specific pointer type
%ixdefine ptr_t_reserve		ptrdiff_t_reserve
%ixdefine float_s_reserve	__nxfloat_reserve	; 32-bit float reservation
%ixdefine float_s			dword
%ixdefine float_size		__nxfloat_size
%ifidn __BITS__,64
	%ixdefine ptr_t_size	qword_size
	%ixdefine dummy_addr	0x175ABADADD		; 40-bit address just to make sure we overflow a dword
%else
	%ixdefine ptr_t_size	dword_size
	%ixdefine dummy_addr	0x15BADADD		; 32-bit address just to make sure we utilize a whole dword
%endif

; libparticlasm declarations generated from the C header
;%include "libparticlasm.inc"
{%s}

; =============================================================================
; BOILERPLATE ENDS HERE
; =============================================================================

[list +]
