.MODEL SMALL
.STACK 64
.DATA
        DEC_STR  DB  20 DUP('$') ;
        HEX_STR  DB  20 DUP('$') ;
        NUMBER_TEN   DW 10  ;
        NUMBER_16   DW 16  ;
        RAW_RESULT  DW  0   ; 
        IS_HEX  DW  0   ; 
        OPERATOR  DW  0   ; 
        DIVIDE_BY_ZERO_ERROR  DB 'ERROR -> The denominator must be the opposite of zero ', '$'
.CODE  

MAIN    PROC FAR
        MOV AX,@DATA
        MOV DS,AX
        MOV CX, 0
        MOV BX, 0
        MOV DX, 0
        MOV AH, 1                               ; read a character
        INT 21H

WHILE_LOOP :
        CMP AL, 13                              ;End of read
        JBE END_WHILE_LOOP                   ;jump out of WHILE_LOOP 
READ_H :
        CMP AL, 104  ; H ENTERED ! MUST CALCULATE HEXADECIMAL OF INPUT ANS DAVE IT 
	JE HEX_NUMBER

        ; Check that the operator has entered 
        CMP AL, 42                              ;OP is *
        JNE PLUS_OP
        MOV CH, 1                               ;Save op [1 = *]
        MOV CL, 0                               ;For read first digit of second number
        JMP SET_INT     


PLUS_OP :       
        CMP AL, 43                              ;OP is +
        JNE MINUS_OP    
        MOV CH, 2                               ;Save op [2 = +]
        MOV CL, 0                               ;For read first digit of second number
        JMP SET_INT     

MINUS_OP :      
        CMP AL, 45                              ;OP is -
        JNE DIVIDE_OP   
        MOV CH, 3                               ;Save op [3 = -]
        MOV CL, 0                               ;For read first digit of second number
        JMP SET_INT     

DIVIDE_OP :
        CMP AL, 47                              ;OP is /
        JNE DIGIT_READ  
        MOV CH, 4                               ;Save op [4 = /]
        MOV CL, 0                               ;For read first digit of second number
        JMP SET_INT     

DIGIT_READ :    
        SUB AL, 48 
        CMP AL, 9
        JBE NOT_HEX
        SUB AL, 39                              ;A B C D E F
NOT_HEX :     
        CMP CH, 0                               ;Check which number is entered
        JNE READ_SECONDE_NUMBER 
        CMP CL, 0                               ;Check which digit is entered 
        JE READ_FIRST_DIG_OF_FIRST_NUM
        JNE READ_SEC_DIG_OF_FIRST_NUM

READ_FIRST_DIG_OF_FIRST_NUM :
        MOV BL, AL
        MOV BH, AL
        INC CL
        JMP SET_INT

READ_SEC_DIG_OF_FIRST_NUM :
        MOV DH, AL ;SAVE SEC DIGI FOR BCD [IF INP IS HEX WE WILL NEED RAW INP]
        MOV AL, BL
        MOV CL, 10
        MUL CL
        MOV BL, AL
        ADD BL, DH
        MOV CL, 4
        SHL BH, CL
        OR BH, DH
        JMP SET_INT

READ_SECONDE_NUMBER :
        CMP CL, 0                       ;Check which digit is entered 
        JE READ_FIRST_DIG_OF_SEC_NUM
        JNE READ_SEC_DIG_OF_SEC_NUM

READ_FIRST_DIG_OF_SEC_NUM :
        MOV DL, AL
        MOV DH, AL
        INC CL
        JMP SET_INT

READ_SEC_DIG_OF_SEC_NUM :
        MOV BH, AL
        MOV AL, DL
        MOV CL, 10
        MUL CL
        MOV DL, AL
        ADD DL, BH
        MOV CL, 4
        SHL DH, CL
        OR DH, BH
        JMP SET_INT


HEX_NUMBER :
        MOV CH, 1
        MOV SI, OFFSET IS_HEX
        MOV [SI], CH  
        INC SI
        CMP CH, 0
        JE CALC_HEX_OF_SEC_NUMB
        CMP BL, 9
        JBE SET_INT
        MOV AL, BH
        MOV CL, 4
        SHR AL, CL
        MOV CL, 16
        MUL CL
        MOV CL, 0
        AND BH, 0Fh
        ADD AL, BH
        MOV BL, BH
        JBE SET_INT

CALC_HEX_OF_SEC_NUMB :
        CMP DL, 9
        JBE SET_INT
        MOV AL, DH
        MOV CL, 4
        SHR AL, CL
        MOV CL, 16
        MUL CL
        MOV CL, 0
        AND DH, 0Fh
        ADD AL, DH
        MOV DL, DH
        JBE SET_INT




SET_INT :
        MOV AH, 1                       ; read a character
        INT 21H
        JMP WHILE_LOOP

;OPERAND1 = BL 
;OPERAND2 = DL
END_WHILE_LOOP :
        MOV CL, 0
        MOV AH, 0
        CMP CH, 1                       ;OP is *
        JNE PLUS
        MOV AL, BL
        MUL DL
        MOV CH, 0
        JMP MAKE_RESULT_DECIMAL

PLUS :
        CMP CH, 2                       ;OP is +
        JNE MINUS
        MOV AL, BL
        ADD AL, DL
        MOV AH, 0
        MOV CH, 0
        JMP MAKE_RESULT_DECIMAL

MINUS :
        CMP CH, 3                       ;OP is -
        JNE DIVIDE
        MOV AL, BL
        CMP AL, DL
        JL RESULT_IS_NEG
        SUB AL, DL  
        MOV AH, 0
        MOV CH, 0
        JMP MAKE_RESULT_DECIMAL

RESULT_IS_NEG :
        SUB DL, AL  
        MOV AH, 0
        MOV CH, 1 ;RESULT IS NEG ! SO WE MUST PRINT - AT FIRST
        JMP MAKE_RESULT_DECIMAL

DIVIDE : 
        CMP DL, 0
        JNE VALID_DIV
        LEA DX, DIVIDE_BY_ZERO_ERROR            
        MOV AH, 9            
        INT 21H	
	JMP END_OF_PROGRAM
VALID_DIV : 
        MOV AH, 0
        CMP CH, 4                       ;OP is /
        CMP DX, 0   
        MOV AL, BL
        DIV DL
        MOV AH, 0
        MOV CH, 0
        JMP MAKE_RESULT_DECIMAL

; Reslut in AX
MAKE_RESULT_DECIMAL : 
        MOV RAW_RESULT, AX
	MOV BX, 0
        MOV CL, 4
        ;RIGHT MOST DIGIT OF RESULT
	CMP AX, 0
	JE MAKE_DEC_STR
	MOV DX, 0
	DIV NUMBER_TEN
	MOV BL, DL
  
	CMP AX, 0
	JE MAKE_DEC_STR
	MOV DX, 0
	DIV NUMBER_TEN
	SHL DL, CL
	OR BL , DL
	
	CMP AX, 0
	JE MAKE_DEC_STR
	MOV DX, 0
	DIV NUMBER_TEN
	MOV BH , DL
	;LEFT MOST DIGIT OF RESULT
	CMP AX, 0
	JE MAKE_DEC_STR
	MOV DX, 0
	DIV NUMBER_TEN
	SHL DL, CL
	OR BH , DL

MAKE_DEC_STR :	  
        MOV AX, BX
	MOV SI, OFFSET DEC_STR
        CMP CH, 1               ;CHECK SIGN OF RESULT
        JNE FOURTH_DIG_OUT
        MOV CH, 45 
        MOV [SI], CH            ; PRINT - AT FIRST
        MOV CH, 1
        INC SI
FOURTH_DIG_OUT : 
        MOV CL, AH
        SHR CL, 1 
        SHR CL, 1 
        SHR CL, 1 
        SHR CL, 1 
        CMP CL, 0
        JE THIRD_DIG_OUT
        ADD CL, 48
        MOV [SI], CL
        INC SI
	AND AH, 0FH
	JMP THIRDNUM
THIRD_DIG_OUT :  
        AND AH, 0FH
	CMP AH, 0
	JE SECOND_DIG_OUT
THIRDNUM :     
        ADD AH, 48
        MOV [SI], AH
        INC SI
	MOV CL, AL
	SHR CL, 1 
	SHR CL, 1 
	SHR CL, 1 
	SHR CL, 1 		  
	JMP SECONDNUM
SECOND_DIG_OUT :  	  
        MOV CL, AL
	SHR CL, 1 
	SHR CL, 1 
	SHR CL, 1 
	SHR CL, 1        
	CMP CL, 0
	JE FIRST_DIG_OUT
SECONDNUM :	  
        ADD CL, 48
        MOV [SI], CL
        INC SI
FIRST_DIG_OUT :        
        AND AL, 0FH
        ADD AL, 48
        MOV [SI], AL
        INC SI               


MAKE_RESULT_HEX : 
        MOV AX, IS_HEX                  ;CHECK FOR Need to print hex?
        CMP AX, 0
        JE PRINT_DEC_STR
        
        MOV SI, OFFSET HEX_STR          ;for save hex result
        MOV AX, RAW_RESULT              

	MOV BX, 0
	CMP AX, 0
	JE MAKE_HEX_STR
	MOV DX, 0
	DIV NUMBER_16 
	MOV BL , DL

	CMP AX, 0
	JE MAKE_HEX_STR
	MOV DX, 0
	DIV NUMBER_16 
        MOV CL, 4
	SHL DL, CL
	OR BL , DL
	
	CMP AX, 0
	JE MAKE_HEX_STR
	MOV DX, 0
	DIV NUMBER_16 
	MOV BH , DL
	
	CMP AX, 0
	JE MAKE_HEX_STR
	MOV DX, 0
	DIV NUMBER_16 
        MOV CL, 4
	SHL DL, CL
	OR BH , DL

MAKE_HEX_STR :	  
	MOV AX, BX
        CMP CH, 1
        JNE CHECK_FOURTH_DIG_OF_HEX
        MOV CH, 45 
        MOV [SI], CH                            ; IF RESULT IS NEGATIVE WE MUST PRINT - AT FIRST
        INC SI
CHECK_FOURTH_DIG_OF_HEX :    
        MOV CL, AH
        SHR CL, 1 
        SHR CL, 1 
        SHR CL, 1 
        SHR CL, 1 
        CMP CL, 0
        JE CHECK_THIRD_DIG_OF_HEX
        ADD CL, 48
        CMP CL, 57 
        JBE WRITE_FOURTH_DIG_TO_HEX_STR
        ADD CL, 39                              ;A B C D E F 
WRITE_FOURTH_DIG_TO_HEX_STR :     
        MOV [SI], CL
        INC SI
	AND AH, 0FH
	JMP THIRD_OF_HEX
CHECK_THIRD_DIG_OF_HEX :                              ;CHECK IS DIGIT3 ZERO?
        AND AH, 0FH
	CMP AH, 0
	JE CHECK_SECOND_DIG_OF_HEX
THIRD_OF_HEX :   
        ADD AH, 48
        CMP CL, 57 
	JBE WRITE_THIRD_DIG_TO_HEX_STR
	ADD CL, 39
WRITE_THIRD_DIG_TO_HEX_STR :		  
        MOV [SI], AH
        INC SI
	MOV CL, AL
	SHR CL, 1 
	SHR CL, 1 
	SHR CL, 1 
	SHR CL, 1 		  
	JMP SECOND_OF_HEX
CHECK_SECOND_DIG_OF_HEX :  	  
        MOV CL, AL
	SHR CL, 1 
	SHR CL, 1 
	SHR CL, 1 
	SHR CL, 1        
	CMP CL, 0
	JE FIRST_DIG_OF_HEX
SECOND_OF_HEX :  
        ADD CL, 48
        CMP CL, 57 
	JBE WRITE_SECOND_DIG_TO_HEX_STR
	ADD CL, 39
WRITE_SECOND_DIG_TO_HEX_STR :		  
        MOV [SI], CL
        INC SI
FIRST_DIG_OF_HEX :      
        AND AL, 0FH
        ADD AL, 48
        CMP CL, 57 
	JBE WRITE_FIRST_DIG_TO_HEX_STR
	ADD CL, 39
WRITE_FIRST_DIG_TO_HEX_STR :		  
        MOV [SI], AL
        INC SI
	MOV AX, 104
	MOV [SI], AX
	INC SI

PRINT_HEX_STR : 
        MOV DX, OFFSET HEX_STR          ;Print hex result
        MOV AH, 9
        INT 21H

PRINT_DEC_STR :                         ;Print decimal result
        MOV DX, OFFSET DEC_STR
        MOV AH, 9
        INT 21H

END_OF_PROGRAM :
	MOV AH,4CH
        INT 21H
MAIN    ENDP

        END MAIN
