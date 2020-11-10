#
#
#

og = input("Enter the name of the file to be read: ")
cp = input("Enter the name of the file to be written: ")
copy = open(cp, 'w')

with open(og, 'r') as f:
    w_line = [0,0,0,0,0,0,0,0,0,0,0,0]
    for line in f:
        index = 0
        mita = 0
        for i in range(97):
            if (((i+1) % 8) == 0) and i != 0:
                mita = hex(int(line[i-7:i+1],2))
                w_line[index] = str(mita)
                index += 1
        line = ",".join(w_line) + ",\n"
        copy.writelines(line)
