/*
 * payload.S
 *
 *  Created on: 10. 5. 2024
 *      Author: ladmanj
 */

.global app_a
.global app_b

.section .app_a,"ax"
app_a:
	.incbin "Core/Src/modbule_a.bin"
.section .app_b,"ax"
app_b:
	.incbin "Core/Src/modbule_b.bin"
