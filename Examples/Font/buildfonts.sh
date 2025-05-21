#!/bin/bash

L_ASCII_RANGES='-r32-255'
L_UNICODE_RANGES='-r32-255,8352-8399'
L_NUMBER_RANGES='-r48-57,43-46,'

fontcvt TtfFonts/ArialUni.ttf ${L_ASCII_RANGES} -s14 -o 'arialuni14'
fontcvt TtfFonts/ArialUni.ttf ${L_ASCII_RANGES} -s18 -o 'arialuni18'
