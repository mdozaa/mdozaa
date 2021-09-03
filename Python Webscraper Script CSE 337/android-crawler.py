"""
Miguel Pacheco
111453454
CSE337 Extra Credit
"""
import requests
from bs4 import BeautifulSoup
import os
# Need a directory for files to be saved in:
if not os.path.exists('outFiles'):
    os.makedirs('outFiles')
# Next, go get the base page to scour through for the links of the APIs
basePage = requests.get('https://developer.android.com/reference/android/app/package-summary')
soup = BeautifulSoup(basePage.text, "html.parser")
baseURL = 'https://developer.android.com'
linksAPI = []
# We are targeting td tags: these contain the a tags with the href for the APIs
for td in soup.find_all('td'):
    for link in td.find_all('a'):
        url = link.get('href')
        # some links found through td tags need to still be filtered
        #
        if url and '/reference/android/' in url and 'https:' not in url and '#' not in url:
            linksAPI.append(url)
fullLinkAPI = []
# Lets add the base url to the hrefs and put all these links in one list
for addendum in linksAPI:
    fullLinkAPI.append(baseURL + addendum)
# ---------------------------------------
# Page Scrape
# Create a dictionary that will store APIs and their methods that contain notes/warnings
APIandDictionary = {}
# Standard counter to use in while loop
counter = 0
# Iterate through
while counter < len(fullLinkAPI):
    # "Load" the page
    page = requests.get(fullLinkAPI[counter])
    # Use BS4 to parse extract html from the page
    currentPageSoup = BeautifulSoup(page.text, "html.parser")
    # Lets get the name of the current API we are working with
    API = currentPageSoup.title.get_text().split()[0]
    """ print(API) - USED FOR TESTING """
    # Create a dictionary
    thisPageDict = {}
    # Go through all div tags that have "data-version-added" attribute, but does not have "id" attribute
    for div in currentPageSoup.find_all('div', attrs={'data-version-added': True, 'id': False}):
        for h3 in div.find_all('h3', {'class': 'api-name'}):
            """ print(h3['data-text']) - USED FOR TESTING """
            listofP = []
            for p in div.find_all('p', attrs={'class': True}):
                textInP = p.get_text().split()
                fixedText = " ".join(textInP)
                listofP.append(fixedText)
            if len(listofP) > 0:
                thisPageDict[h3['data-text']] = listofP

    if len(thisPageDict) > 0:
        APIandDictionary[API] = thisPageDict
        # print("FLAG") - USED FOR TESTING
    # print("END") - USED FOR TESTING
    counter += 1

for API in APIandDictionary:
    fileForFlaggedAPI = open(API + ".txt", "w")
    foundDict = APIandDictionary[API]
    #print(API)
    for key in foundDict:  # founddict is thispageDICT
        for value in foundDict[key]:
                returnThisString = "- " + key + ":" + value
                #print(returnThisString)
                fileForFlaggedAPI.write(returnThisString + '\n')

    fileForFlaggedAPI.close()







