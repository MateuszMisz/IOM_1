import sys
# print('wklej tableke')
table = sys.stdin.read()
lines = table.split('\n')
colnames = lines[0].split()
output_lines=[]
output_lines.append(';'.join(colnames))
for index,line in enumerate(lines[2:]):

    for idx,c in enumerate(line):
        if c==')':
            split_idx2=idx+1
            break
    # print(line.split())
    first = line.split()[0]
    second = "".join(line[:split_idx2].split()[1:])
    third = line[split_idx2:]
    output_lines.append(';'.join([first, second, third]))
print('\n'.join(output_lines))