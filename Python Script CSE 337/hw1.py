"""
Miguel Pacheco
111453454
CSE 337
Homework 1 - 9/28/2020
"""

def isValid(str):
    separatedstr = [char for char in str]
    thisdict = {}
    maxnum = 0
    nums = []
    occurdict = {}

    # must check that strings of only characters [a-z] are used
    if str.isalpha() is False:
        return 0

    # iterate over string and insert into a dictionary in the format: 'character' : number of occurrences
    for x in separatedstr:
        if x in thisdict:
            thisdict[x] += 1
        else:
            thisdict[x] = 1

    # iterate over the dictionary and extract the values into a list
    # (because we are concerned about the number of occurrences)
    for y in thisdict:
        temp = int(thisdict[y])
        nums.append(temp)
        if temp > maxnum:
            maxnum = temp

    # iterate over the list of occurrences and create a dictionary in the format:
    # (K:V) number of occurrence of character : number of occurrence of occurrence
    for z in nums:
        if z in occurdict:
            occurdict[z] += 1
        else:
            occurdict[z] = 1

    # Since we are evaluating the number of occurrences of occurrences, there should be at max 2 different keys
    if len(occurdict) > 2:
        return 0  # return 0 means invalid
    # If there are two different keys, the keys should only have a difference of 1
    elif len(occurdict) == 2:
        checker = maxnum - list(occurdict)[0]
        if checker != 1:
            return 0
    # If there is only one key, then it's always valid.
    elif len(occurdict) == 1:
        return 1

def isBalanced(str):
    # predefining proper relationships and definitions
    bracketpairDict = {"{": "}", "[": "]", "(": ")"}
    openBrackets = ["{", "[", "("]
    closingBrackets = ["}", "]", ")"]

    openingBracketList = []
    closingBracketList = []

    separatedstr = [char for char in str]

    # Iterate through list and sorting each character by open or close
    for x in separatedstr:
        if x in openBrackets:  # open brackets sorted here
            openingBracketList.append(x)
        if x in closingBrackets:  # closing brackets sorted here
            closingBracketList.append(x)

    # openingBracketList and closingBracketList should always be mirrored (1st in oBL is last in cBL); reverse list
    closingBracketList = list(reversed(closingBracketList))

    if len(openingBracketList) != len(closingBracketList):
        return 0

    counter = 0
    while counter < len(openingBracketList):
        if bracketpairDict[openingBracketList[counter]] == closingBracketList[counter]:
            counter += 1
        else:
            return 0

    return 1


class Node:

    def __init__(self, name, nodeL=None , nodeR=None):  # should nodeL by input as a String?
        self.name = name
        self.left = None
        self.right = None
        self.order = []
        # print("Node: ", name, " created")

        # nodeL comes in as Node(x), where x is the label; must extract the x

        if nodeL is not None:
            self.left = nodeL

        if nodeR is not None:
            self.right = nodeR

    # get the left child of the current node.
    def getLeftChild(self):
        return self.left

    # get the right child of the current node
    def getRightChild(self):
        return self.right

    # getName serves as getData
    def getName(self):
        return self.name

    # helper function; this is what actually traverses the tree
    def preOrderTrav(self, node):
        res = []
        if node:
            res.append(node.getName())
            res = res + self.preOrderTrav(node.left)
            res = res + self.preOrderTrav(node.right)
        return res

    # call preOrder traversal on the current node
    def preOrder(self):
        return self.preOrderTrav(self)

    # helper function; this is what actually traverses the tree
    def inOrderTrav(self, node):
        res = []
        if node:
            res = self.inOrderTrav(node.left)
            res.append(node.getName())
            res = res + self.inOrderTrav(node.right)
        return res

    # call inOrder traversal on the current node
    def inOrder(self):
        return self.inOrderTrav(self)

    # helper function; this is what actually traverses the tree
    def postOrderTrav(self, node):
        res = []
        if node:
            res = self.postOrderTrav(node.left)
            res = res + self.postOrderTrav(node.right)
            res.append(node.getName())
        return res

    # call postOrder traversal on the current node
    def postOrder(self):
        return self.postOrderTrav(self)

    # get the sum of the tree
    # recall that traversals all return a list [] with integers as names
    def sumTree(self):
        res = self.inOrderTrav(self)
        treeSum = 0
        for x in res:
            treeSum += x
        return treeSum


# First Problem driver
evaluateThis = input("Enter the string to evaluate: ")
result = isValid(evaluateThis)
if result == 0:
    print("Not valid")
else:
    print("Valid")

# Second problem driver
evaluateThis2 = input("Enter the string of brackets to evaluate: ")
result = isBalanced(evaluateThis2)
if result == 0:
    print("Not valid")
else:
    print("Valid")

# Third and fourth problem driver
evaluateThis3 = input("Enter your tree: ")
root = eval(evaluateThis3)

if root is not None:
    print("Preorder: ", root.preOrder())
    print("Inorder: ", root.inOrder())
    print("Postorder: ", root.postOrder())
    print("Tree sum: ", root.sumTree())
else:
    print("Preorder: [] ")
    print("Inorder: []")
    print("Postorder: []")
    print("Tree sum: 0")
