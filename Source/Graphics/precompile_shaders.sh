#!/bin/bash
echo "Start compiling shaders"
shopt -s nullglob
for filename in Shaders/*.glsl; 
do
	echo "process $filename"
	new_file="${filename/.glsl/.spv}"
	glslc.exe "$filename" "-o" "$new_file"
done

for filename in Shaders/*.vert; 
do
	echo "process $filename"
	new_file="${filename/.vert/_vert.spv}"
	glslc.exe "$filename" "-o" "$new_file"
done

for filename in Shaders/*.frag; 
do
	echo "process $filename"
	new_file="${filename/.frag/_frag.spv}"
	glslc.exe "$filename" "-o" "$new_file"
done

for filename in Shaders/*.geom; 
do
	echo "process $filename"
	new_file="${filename/.geom/_geom.spv}"
	glslc.exe "$filename" "-o" "$new_file"
done

read -p "Press any key to resume ..."
