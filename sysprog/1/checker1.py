with open("out.txt", 'r') as f:
    data = f.read()

is_ok = True
data = data.split()
prev_number = -(1 << 31 - 1)
for i in range(0, len(data)-100):
    # print(data[i])
    try:
        v = int(data[i])
        if v < prev_number:
            is_ok = False
            print('Error on numbers {} {}, position in file - {}'.format(prev_number, v, i))
        prev_number = v
    except:
        pass
if is_ok:
    print('All is ok')
