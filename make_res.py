import os
from sys import argv
from PIL import Image
from shutil import copyfile

#запуск данного файла приведет к созданию ресурсного файла для прошивки.
#Имена картинок и их размер x*y для должны строго совпадать с теми, что присутствуют на данный момент
#Создавая собственный дизайн прошивки, пожалуйста, указывайте на титульной картинке свое авторство и кодовое имя прошивки SHUI
#Базовый набор картинок будет обновляться в отдельном репозитории на GIT

#В этом же наборе ресурсов присутствует файл со шрифтом h21.fnt правила сборки которого будут опубликованы позднее

#настроить 3 пути
#res_dir - местоположение картинок
#dest_dir - каталог компиляции
#res_file - файл для прошивки RESDUMP.BIN
#project - вспомогательная переменная, выделяющая общую часть пути

#res_dir=argv[1]
project='/home/shubin/electronic/firmware/mks-robin/my/Marlin/'
res_dir=project + 'Marlin/src/lcd/extui/lib/shui/res/img'
dest_dir=project + '.pio/build/img'
res_file=project + '.pio/firmware/RESDUMP.BIN'

files_list=[]


def png_to_565(infn, outfn):
	image=Image.open(infn)
	directory=os.path.dirname(outfn)
	if not os.path.exists(directory):
		os.makedirs(directory);
	out_file = open(outfn, "wb")
	data=image.getdata()

	for d in data:
		b1=(d[2]>>3) | ((d[1]<<3) & 0xe0)
		b2=(d[0] & 0xF8) | (d[1]>>5)
		out_file.write(bytearray([b1, b2]))

	out_file.close()

def copy_proc(infn, outfn):
  directory=os.path.dirname(outfn)
  if not os.path.exists(directory):
    os.makedirs(directory);
  copyfile(infn, outfn)

def walk(directory):
	for d, dirs, files in os.walk(directory):
		for f in files:
			files_list.append(os.path.join(d, f))

crc=0


def make_dump(out_file_name):
	print("Files count:", len(files_list))
	out_file = open(out_file_name, "wb")
	global crc	
	crc=0
	
	def to_crc(data):
		global crc	
		for d in data:
			crc = crc ^ d

	def out_write(data):
		to_crc(data)
		out_file.write(data)
		
	offset=0
	out_write('SHUI'.encode('utf8'))											#4 - Сигнатура
	out_file.write(offset.to_bytes(4, byteorder="little"))	#4 - Общий размер файла
	out_file.write(offset.to_bytes(4, byteorder="little"))	#4 - Смещение образов файлов
	out_write(len(files_list).to_bytes(1, byteorder="little"))	#1 - Число файлов
	offset+=13
	for file_name in files_list:
		relative=os.path.relpath(file_name,dest_dir)
		size=os.path.getsize(file_name)
		name_bytes = relative.encode('cp1251')		
		out_write(len(name_bytes).to_bytes(1, byteorder="little"))
		out_write(name_bytes)
		out_write(size.to_bytes(4, byteorder="little"))
		offset+=len(name_bytes)+5
		print(size, "\t", relative)


	for file_name in files_list:
		in_file = open(file_name, "rb")
		out_write(in_file.read())
		in_file.close()

	total = out_file.tell()+1
	out_file.seek(4)
	out_write(total.to_bytes(4, byteorder="little"))
	out_write(offset.to_bytes(4, byteorder="little"))
	
	out_file.seek(0, 2)
	out_file.write(crc.to_bytes(1, byteorder="little"))
	out_file.close()

def check_dump(read_file_name):
	global crc
	crc=0
	in_file = open(read_file_name, "rb")
	for d in in_file.read():
		crc=crc^d
	print("CRC:", crc)

print("deleting")
for root, dirs, files in os.walk(dest_dir, topdown=False):
  for name in files:
    os.remove(os.path.join(root, name))
  for name in dirs:
    os.rmdir(os.path.join(root, name))

for d, dirs, files in os.walk(res_dir):
	for f in files:
		in_file=os.path.join(d, f)
		rel_file=os.path.relpath(in_file, res_dir)
		fn, fe = os.path.splitext(rel_file)
		proc=0
		if (fe==".png"):
			out_rel_file=fn+".565"
			proc=png_to_565
		elif (fe==".fnt"):
			out_rel_file=rel_file
			proc=copy_proc
		else:
			proc=copy_proc
			out_rel_file=rel_file
		out_file=os.path.join(dest_dir, out_rel_file)
		proc(in_file, out_file)


walk(dest_dir)
make_dump(res_file)
check_dump(res_file)

