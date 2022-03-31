import math

def softmax(v):
	sum = 0
	for x in v:
		sum += math.exp(x)
	ret = []
	for x in v:
		ret.append(math.exp(x)/sum)
	return ret

def logsoftmax(v):
	maxv = max(v)
	sum = 0
	for x in v:
		sum += math.exp(x - maxv)
	sum = math.log(sum)
	ret = []
	for x in v:
		ret.append(x - maxv - sum)
	return ret

print(softmax([90,89]))
print(logsoftmax([90,89]))
