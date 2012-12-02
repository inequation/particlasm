%ifidn __BITS__,32
	; sort of an EIP-relative addressing hack
	call	.fake_label
.fake_label:
	pop		__si
	sub		__si, (.fake_label - __Origin)
%endif
