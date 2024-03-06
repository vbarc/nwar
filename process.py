import fileinput

for line in fileinput.input():
    line = line.strip()
    if " = VK_" in line:
        continue
    fmt = line.split()[0]
    fmt_abbr = fmt[15:]
    print(f'        case {fmt}:')
    print(f'            return "{fmt_abbr}";')
