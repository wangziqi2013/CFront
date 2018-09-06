
fp = open("token_list.txt", "r")
s = fp.read()
l = s.split(",")
for w in l:
	w = w.strip()
	if w == "": continue
	print "    case %s: return \"%s\";" % (w, w,)
