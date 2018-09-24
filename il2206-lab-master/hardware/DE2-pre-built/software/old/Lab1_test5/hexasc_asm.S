        # stub for lab 1, task 2.4
        
        .global hexasc          # makes label "hexasc" globally known
        
        .data                   # area for data - not needed here!

        .text                   # area for instructions
		.global hexasc

hexasc: andi	r4,0xF		 # write your assembler code here
		cpi		r4,0xA
		brlo	number

letter:	subi r4,0xA
		movi r2,0x41
		br end

number:	movi 	r2,0x31

end:	add r2,r4
		return
#        .end                   # The assembler will stop here
