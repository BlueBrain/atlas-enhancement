BEGIN { printf "View \"points\" {\n" }
{ printf "SP(%s,%s,%s){0};\n",$1,$2,$3 }
END { printf "};\n" }
