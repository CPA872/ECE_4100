f = open("mm_tiled.csv")

for line in f.readlines()[2:]:
    arr = line.split(",")
    for a in arr:
        a.strip()
    c, b, s, rp, v, aat = arr
    c = int(c)
    b = int(b)
    s = int(s)
    v = int(v)
    aat = float(aat)

    if b == 6 and s == 0 and 'LRU' in rp:
        print(c, b, s, rp, v, aat, sep=",")
