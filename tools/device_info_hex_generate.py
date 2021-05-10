'''
用于生成量产hex
'''

# 全局设置，单片机内保存设备信息的地址
BASE_ADDR = 0x0801b000

BASE_ADDR_H = (BASE_ADDR >> 16) & 0xFFFF
BASE_ADDR_L = BASE_ADDR & 0xFFFF

hex_index = ['0', '1', '2', '3',
            '4', '5', '6', '7',
            '8', '9', 'A', 'B',
            'C', 'D', 'E', 'F']

rectype = {'00' : "Data Record",
            '01' : "End of File Record",
            '02' : "Extended Segment Address Record",
            '03' : "Start Segment Address Record",
            '04' : "Extended Linear Address Record",
            '05' : "Start Linear Address Record"}
            

def uint8_to_hex_str(data):
    # int_data = to_bytes(4, byteorder='little', signed=True)
    int_data = data.to_bytes(1, byteorder='big', signed=False)
    return int_data.hex().upper()


def uint16_to_hex_str(data):
    # int_data = to_bytes(4, byteorder='little', signed=True)
    int_data = data.to_bytes(2, byteorder='big', signed=False)
    return int_data.hex().upper()


# hex文件的校验和是所有数据加起来为0
def hex_check_sum(dat_str):
    hex_data = bytes.fromhex(dat_str)
    chk_sum = 0
    for each_byte in hex_data:
        chk_sum += int(each_byte)
    chk_sum = chk_sum & 0xFF
    if (chk_sum != 0):
        return 256 - chk_sum
    else:
        return 0


def gen_line(addr, type, data):
    out_line = ""
    chk_sum = 0
    # bytes_data = bytes(data, encoding="ascii")
    # bytes_len = len(bytes_data)
    bytes_len = len(data)
    out_line += uint8_to_hex_str(bytes_len)
    out_line += uint16_to_hex_str(addr)
    # out_line += uint8_to_hex_str(0x00) # 数据类型，全部为 Data Record
    out_line += uint8_to_hex_str(type) # 数据类型
    # out_line += bytes_data.hex()
    out_line += data.hex()
    chk_sum = hex_check_sum(out_line)
    out_line = ":" + out_line + uint8_to_hex_str(chk_sum)
    out_line += "\n"
    # print(out_line)
    return out_line


def gen_hex(product_id, device_name, device_psk):
    filename = device_name + ".hex"
    print("generate file:", filename)
    with open(filename, 'w', encoding='ascii') as f:
        # 设置扩展数据段地址
        f.write(gen_line(0x0000, 4, BASE_ADDR_H.to_bytes(2, byteorder='big', signed=False)))
        # product id
        f.write(gen_line(BASE_ADDR_L + 0, 0, bytes(product_id + "\0", encoding="ascii")))
        # device name
        f.write(gen_line(BASE_ADDR_L + 0x40, 0, bytes(device_name + "\0", encoding="ascii")))
        # device psk
        f.write(gen_line(BASE_ADDR_L + 0x80, 0, bytes(device_psk + "\0", encoding="ascii")))
        # 结束
        f.write(":00000001FF" + "\n")


def main():
    with open("device_info.txt", 'r', encoding='ascii') as f:
        for each_line in f:
            if (len(each_line) < 2):
                break
            dev_info = each_line.split(" ")
            gen_hex(dev_info[0], dev_info[1], dev_info[2])
    print("finish")
    # gen_hex("001", "dev_name", "this is psk")
    # data_to_hex_str(bytes("12345", encoding="ascii"))
    # gen_line(0x1234, 1, "abcd1234")


if __name__ == '__main__':
    main()
