// Game "Steelrat" for 8 bit computer "Apogey BK01C"
// 1985 English text (c) Harry Harrison
// 2014-07-25  Build on Windows & tasm version, Alemorf, aleksey.f.morozov@gmail.com
// 2021-01-05  Build on Linux & sjasm version,  Alemorf, aleksey.f.morozov@gmail.com

        device zxspectrum48
        org 0

begin:
        ; Очищаем экран
        ld   c, 1Fh
        call 0F809h
	
        ; Начальная глава
        xor  a
        ld   (curChapter), a

        ; Скрываем курсор
        call hideCursor

        ; Рисуем рамку
        ld   bc, screenS
        ld   hl, 0E2C1h
        call prn
        call clearCenter
        ld   bc, screenE
        call prn

        ; Разархивация главы
loadChapter:
        ld   hl, (curChapter)
        ld   h, 0
        add  hl, hl
        ld   de, map
        add  hl, de
        ld   e, (hl)
        inc  hl
        ld   d, (hl)
        ld   bc, buffer
        call unmlz

        ; Первая страница в главе
        ld   c, 0

        ; Поиск страницы
loadPage:
        ld  hl, buffer
        ; Переход на следующую главу
        ld  a, c
        cp  254
        jp  z, nextChapter
        ; Поиск страницы
        inc c
loadPage_1:
        dec c
        jp  z, drawPage
loadPage_2:
        ld a, (hl)
        inc hl
        or  a
        jp  nz, loadPage_2
        ld  a, (hl)
        inc hl
        cp  0FFh
        jp  nz, loadPage_2
        jp  loadPage_1

beep:	; Би-би
        ld   c, 7
        call 0F809h

hideCursor:
        ; Скрываем курсор
        push hl
        ld   hl, 0EF01h
        ld   (hl), 80h
        dec  hl
        ld   (hl), 0FFh
        ld   (hl), 0FFh
        pop  hl
        ret

;----------------------------------------------------------------------------

clearCenter:
        ld   hl, 0E30Fh
        ld   d, 23
clearCenter_1:
        push de
        ld   bc, screenL
        call prn
        pop  de
        dec  d
        jp   nz, clearCenter_1
        ret

;----------------------------------------------------------------------------
		
drawPage:
        push hl
        call clearCenter
        pop  bc

        ld   hl, 0E360h
        call prn

        ld   de, ptrs
drawPage_1:
        ld   a, (bc)
        inc  bc
        cp   0FFh
        jp   z, drawPage_2

        ld   (de), a
        inc  de

        push de
        push bc
        ld   bc, mark
        call prn
        pop  bc
        call prn_l
        pop  de

        jp   drawPage_1

;----------------------------------------------------------------------------

drawPage_2:
        ld  a, -1
        ld  (cursorN), a
        ld  hl, 0E2C0h
        ld  bc, 1
        jp  move_1

;----------------------------------------------------------------------------

nextChapter:
        ld  hl, curChapter
        inc (hl)
        jp  loadChapter

;----------------------------------------------------------------------------

loop:  	ld   hl, (cursorPos)
        ld   a, 9
        cp   (hl)
        jp   nz, loop_5
        xor  a
loop_5:	ld   (hl), a
        ld   hl, 3000h
wait_2:	dec  l
        jp   nz, wait_2
        dec  h
        jp   nz, wait_2

        call 0F81Bh
        cp   0FFh
        jp   z, loop
        call beep

        ld   hl, 5000h
wait_0:
        dec  l
        jp   nz, wait_0
        dec  h
        jp   nz, wait_0
        cp   19h
        jp   z, left
        cp   1Ah
        jp   z, right
        cp   'Q'
        jp   z, nextChapter
        cp   1Bh
        jp   z, begin
        cp   0Dh
        jp   z, enter
        jp  loop

;----------------------------------------------------------------------------

enter:	ld   hl, (cursorN)
        ld   h, 0
        ld   de, ptrs
        add  hl, de
        ld   c, (hl)
        jp   loadPage

;----------------------------------------------------------------------------

left:	ld   bc, -1
        jp   move

;----------------------------------------------------------------------------

right:	ld  bc, 1

move:	ld   hl, (cursorPos)
        ld   (hl), '*'
move_1: add  hl, bc
        ld   a, h
        cp   0E1h
        jp   z, move_2
        cp   0EBh
        jp   z, move_2
        ld   a, (hl)
        cp   '*'
        jp   nz,move_1
        ld   a, (cursorN)
        add  c
        ld   (cursorN), a
move_3: ld   (hl), 9
        ld   (cursorPos), hl
        jp   loop

;----------------------------------------------------------------------------

move_2:	ld   hl, (cursorPos)
        jp   move_3

;----------------------------------------------------------------------------

prn_e:	ld   de, 78
        add  hl, de
prn:	ld   d, h
        ld   e, l
prn_l:	ld   a, (bc)
        inc  bc
        cp   0
        ret  z
        cp   10
        jp   z, prn_e
        ld   (de), a
        inc  de
        jp   prn_l

;----------------------------------------------------------------------------

        include "unmlz.inc"

;----------------------------------------------------------------------------

screenS:    .db 094h," ","\\/\\/\\/\\/\\/\\/\\/\\/\\/\\/ stanx stalxnoj krysoj \\/\\/\\/\\/\\/\\/\\/\\/\\/\\ ",80h,10,0 ;0
screenL:    .db 094h,' ',80h,"                                                            ",94h,' ',80h,10,0 ;1
screenE:    .db 094h," "," ESC - sna~ala \\/\\/\\/\\/\\/\\ 2014-2021 garri garrison & ALEMORF  ",80h,0,0 ; 24
mark:	    .db 10, 10, "* ",0
curChapter: .db 0
cursorPos:  .dw 0E2C0h
cursorN:    .db 0
ptrs:       .db 0,0,0,0

map     dw page00
        dw page01
        dw page02
        dw page03
        dw page04
        dw page05
        dw page06
        dw page07
        dw page08
        dw page09
        dw page10
        dw page11
        dw page12
        dw page13
        dw page14

page00: .incbin "build/00.bin"
page01: .incbin "build/01.bin"
page02: .incbin "build/02.bin"
page03: .incbin "build/03.bin"
page04: .incbin "build/04.bin"
page05: .incbin "build/05.bin"
page06: .incbin "build/06.bin"
page07: .incbin "build/07.bin"
page08: .incbin "build/08.bin"
page09: .incbin "build/09.bin"
page10: .incbin "build/10.bin"
page11: .incbin "build/11.bin"
page12: .incbin "build/12.bin"
page13: .incbin "build/13.bin"
page14: .incbin "build/14.bin"

buffer: db 0


