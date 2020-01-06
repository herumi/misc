import sys

def skipSpace(s):
  i = 0
  while i < len(s):
    c = s[i]
    if c != ' ' and c != '\t':
      break
    i = i + 1
  return s[i:]

def parseNumber(s, stack):
  s = skipSpace(s)
  if s == '':
    raise Exception("Number empty")
  if s[0] == '(':
    ns = parseAddSub(s[1:], stack)
    if ns[0] != ')':
      raise Exception("Number", s, ns)
    return ns[1:]
  num = ''
  i = 0
  while i < len(s):
    c = s[i]
    if c not in "0123456789":
      break
    num = num + c
    i = i + 1
  stack.append(('num', num))
  return s[i:]

def parseAddSub(s, stack):
  s = parseMulDiv(s, stack)
  while s != '':
    s = skipSpace(s)
    if s == '':
      break
    if s[0] == '+':
      s = parseMulDiv(s[1:], stack)
      stack.append(('op', '+'))
    elif s[0] == '-':
      s = parseMulDiv(s[1:], stack)
      stack.append(('op', '-'))
    else:
      break
  return s

def parseMulDiv(s, stack):
  s = parseNumber(s, stack)
  while s != '':
    s = skipSpace(s)
    if s == '':
      break
    if s[0] == '*':
      s = parseNumber(s[1:], stack)
      stack.append(('op', '*'))
    elif s[0] == '/':
      s = parseNumber(s[1:], stack)
      stack.append(('op', '/'))
    else:
      break
  return s

def parseExpression(s):
  stack = []
  s = parseAddSub(s, stack)
  s = skipSpace(s)
  if s != '':
    raise Exception("extra string", s)
  return stack

def stackToTree(stack):
  i = 0
  while i < len(stack):
    if len(stack[i]) == 2:
      (k, v) = stack[i]
      if k == 'op':
        t = [v, [stack[i - 2], stack[i - 1]]]
        del stack[i-2:i+1]
        stack.insert(i-2, t)
        i = i - 1
        continue
    i = i + 1
  return stack
      

def main():
  if len(sys.argv) == 1:
    print("parse.py \"formula\"")
    return 1
  s = sys.argv[1]
  print("parse `%s`" % s)
  stack = parseExpression(sys.argv[1])
  print(stack)
  t = stackToTree(stack)
  print(t)

main() 
