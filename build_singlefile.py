import re

filelist = ['bigint_header.h', 'bigint_base.h',
            'bigint_mini.h', 'bigint_dec.h', 'bigint_hex.h']


def readfile(filename, re_beg, re_end):
    f = open(filename, "r")
    in_flag = False
    result = ""
    for line in f:
        if (re_beg.search(line)):
            in_flag = True
        elif (re_end.search(line)):
            in_flag = False
        elif in_flag:
            result += line.replace('    ', '\t')
    f.close()
    if result:
        return result + '\n'
    return ''


def init_re(name):
    beg = re.compile(r"\/\/.*\{" + name + "_b\}")
    end = re.compile(r"\/\/.*\{" + name + "_e\}")
    return (beg, end)


def doclass(name):
    re_beg, re_end = init_re(name)
    result = ""
    for f in filelist:
        s = readfile(f, re_beg, re_end)
        result += s

    return result


def process(name, out):
    r = doclass(name)
    f = open(out, 'w')
    f.write(r)
    f.close()


if __name__ == "__main__":
    process("hex", "single_bigint_hex.h")
    process("hexm", "single_bigint_hexm.h")  # I/O base in 2,4,8,16,32
    process("dec", "single_bigint_dec.h")
    process("decm", "single_bigint_decm.h")  # I/O base 10 only
    process("mini", "single_bigint_mini.h")  # I/O base 10 only
