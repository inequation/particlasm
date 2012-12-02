	; CONSTANT COLOUR DISTRIBUTION
	movups	xmm5, [rel_addr(__Data.Offset{%d})]
	cdistr_mix
