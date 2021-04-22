import os
import math

def export_font(file_name):
  from Marlin.src.lcd.extui.lib.shui.res import font  
  wf = open(file_name, "wb")
  fonts=font.fonts
  font_index = 0
  for font in fonts:
    char_index=[]
    char_data_size=0
    index_size=(font['max']-font['min']+1)*2
    char_ord = font['min']
    for meta in font['chars']:
      char_size=math.ceil(meta['bw']/8)*21+1
      char_index.append(index_size + char_data_size)
      #print("'{0}', {1}".format(chr(char_ord), index_size + char_data_size))
      char_ord+=1
      char_data_size+=char_size
    
    font['char_index']=char_index
    font['char_data_size']=char_data_size
    
    #font['index']
    font_index+=1

  font_index_offset = 0
  for font in fonts:
    font["index"]=1 + len(fonts)*8+font_index_offset
    font_index_offset=font["char_data_size"]+(font['max']-font['min']+1)*2
    
  wf.write(len(fonts).to_bytes(1, byteorder='little'))
  for font in fonts:
    wf.write(font['min'].to_bytes(2, byteorder='little'))
    wf.write(font['max'].to_bytes(2, byteorder='little'))
    wf.write(font['index'].to_bytes(4, byteorder='little'))
  for font in fonts:
    for i in font['char_index']:
      wf.write(i.to_bytes(2, byteorder='little'))
    for meta in font['chars']:
      bw=meta['bw']
      wf.write(bw.to_bytes(1, byteorder='little'))      
      for r in meta['rows']:
        wf.write(r.to_bytes(math.ceil(bw/8), byteorder='big'))

font_out="Marlin/src/lcd/extui/lib/shui/res/img/h21.fnt"
if os.path.isfile(font_out):
  os.remove(font_out)
export_font(font_out)

