#!/bin/bash

alias a2ps='a2ps -v --line-numbers=1 --font-size=9 --copies=1 --sides=2 ' \
'--landscape --chars-per-line=120 --truncate-lines=1 ' \
'--highlight-level=normal -E --media=A4 -2 --printer=printer'

a2ps -o project.ps kos/*.c include/*.h

ps2pdf project.ps project.pdf
