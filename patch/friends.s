.3ds
.thumb

.open "friends.bin", "friends_patched.bin", 0x100000

; Newtendo will replace the dev URL
; https://nasc.dev.nintendowifi.net/ac -> https://nasc.newtendo.net/ac
.org 0x1612C0
  .area 37
    .asciiz "https://nasc.newtendo.net/ac"
  .endarea
.close

; Pretendo replaces the test URL
; https://nasc.test.nintendowifi.net/ac -> https://nasc.pretendo.cc/ac/
.org 0x16129a
  .area 38
    .asciiz "https://nasc.pretendo.cc/ac/"
  .endarea
