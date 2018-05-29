#!/usr/bin/env python3.4

#TODO Error handling, because there is none

import sys
import csv
import re

from html.parser import HTMLParser

class TableParser(HTMLParser):

    def __init__(self):
        super().__init__(self)
        self._path = []
        self.table = []
        self._row = []
        self._content = []

    def handle_starttag(self, tag, attrs):
        self._path.append(tag)

        if tag == 'tr':
            self._row = []
        
        try:
            if tag == 'a':
                for name, value in attrs:
                    if name == "href":
                        result = re.search(r"gid=(.*)\&puid", value)
                        url = result.group(1)
                        self._row.append(url)
        except:
            pass

    def handle_data(self, data):
        if 'td' in self._path:
            self._content.append(data)

    def handle_endtag(self, tag):
        old_tag = self._path.pop()
        #assert old_tag == tag, "Should end this tag"
        if tag == 'td':
            self._row.append("".join(self._content))
            self._content = []
        elif tag == 'tr':
            self.table.append(self._row)
            self._row = []


def main():
    p = TableParser()
    with open(sys.argv[1], encoding="ISO-8859-1") as fd:
        p.feed(fd.read())
    c = csv.writer(sys.stdout)
    c.writerows(p.table)


if __name__ == '__main__':
    main()
