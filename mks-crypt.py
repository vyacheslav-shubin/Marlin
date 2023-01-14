import os

def encrypt_mks(source):
    import sys
    dir = os.path.dirname(source)
    dest = os.path.join(dir + "/Robin_nano35.bin")
    key = [0xA3, 0xBD, 0xAD, 0x0D, 0x41, 0x11, 0xBB, 0x8D, 0xDC, 0x80, 0x2D, 0xD0, 0xD2, 0xC4, 0x9B, 0x1E, 0x26, 0xEB, 0xE3, 0x33, 0x4A, 0x15, 0xE4, 0x0A, 0xB3, 0xB1, 0x3C, 0x93, 0xBB, 0xAF, 0xF7, 0x3E]

    fwfile = open(source, "rb")
    enfile = open(dest, "wb")
    length = os.path.getsize(source)
    position = 0
    try:
        while position < length:
            byte = fwfile.read(1)
            if 320 <= position < 31040:
                byte = chr(ord(byte) ^ key[position & 31])
                if sys.version_info[0] > 2:
                    byte = bytes(byte, 'latin1')
            enfile.write(byte)
            position += 1
    finally:
        fwfile.close()
        enfile.close()

encrypt_mks(".pio/build/RN12/firmware.bin")
