[list -]

; Particlasm emitter code
; Generated automatically, do not modify!

; =============================================================================
; BOILERPLATE STARTS HERE
; =============================================================================

; set up CPU type and output format
CPU {%s}
BITS {%s}
%ixdefine OUTPUT_FORMAT {%s}

; NASMX base include
;%include 'nasmx.inc'
{%s}

; some NASMX supplements
%ixdefine ptr_t				ptrdiff_t			; platform-specific pointer type
%ixdefine float_storage		__nxfloat_storage	; 32-bit float storage
%ixdefine float_s			dword
%ixdefine float_size		__nxfloat_size
%ifidn __BITS__,64
	%ixdefine ptr_t_size	qword_size
%else
	%ixdefine ptr_t_size	dword_size
%endif

; libparticlasm declarations generated from the C header
;%include "libparticlasm.inc"
{%s}

; relative addressing modes for 32 and 64 bits
%ifidn __BITS__,32
	%define rel_addr(x)	__si + x
%else
	%define rel_addr(x)	rel x
%endif

; a macro for calling the external lib functions from within the emitter code
; %1 - function to retrieve
; %2 - intermediate register (usually __ax)
%macro extlib 2
	mov		%2, [__bp + 2 * sizeof(ptr_t) + ptcExtLib.%1]
	call	%2
%endmacro

; flag-based colour component mixing
%macro cdistr_mix 0
	; simple case - both RGB and alpha are to be written
	test	eax, (ptcCF_SetRGB | ptcCF_SetAlpha)
	; parity bit holds information about equality
	jnp		%%rgb
	movaps	xmm5, xmm6
	jmp		%%end
%%rgb:
	test	eax, ptcCF_SetRGB
	jz		%%alpha
	; mask out alpha from the distribution value and add in the original alpha
	lea		__ax, [rel_addr(MASK_RGB)]
	movups	xmm7, [__ax]
	andps	xmm6, xmm7
	andnps	xmm7, xmm5
	addps	xmm6, xmm7
	movaps	xmm5, xmm6
	jmp		%%end
%%alpha:
	test	eax, ptcCF_SetAlpha
	jz		%%end
	; mask out RGB from the distribution value and add in the original RGB
	lea		__ax, [rel_addr(MASK_ALPHA)]
	movups	xmm7, [__ax]
	andps	xmm6, xmm7
	andnps	xmm7, xmm5
	addps	xmm6, xmm7
	movaps	xmm5, xmm6
%%end:
%endmacro

default rel

; =============================================================================
; BOILERPLATE ENDS HERE
; =============================================================================

[list +]

__Origin:
; xmm masks for colour components - used in colour mixing
MASK_RGB		dd	0xFFFFFFFF
				dd	0xFFFFFFFF
				dd	0xFFFFFFFF
				dd	0x00000000
MASK_ALPHA		dd	0x00000000
				dd	0x00000000
				dd	0x00000000
				dd	0xFFFFFFFF
