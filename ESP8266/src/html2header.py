#!/usr/bin/env python3
'''Simple script that converts an HTML file into an includeable header file'''

import sys
import os
import argparse
import html.parser
import textwrap

# raw string literal template
DEST_STR = 'const char *INDEX PROGMEM = R"""({})""";'


class HTMLPacker(html.parser.HTMLParser):
    '''Modified HTML Parser that puts referenced files into the HTML source'''
    def __init__(self, *, convert_charrefs=True):
        # Holds packed data
        self.converted_data = []
        
        super().__init__(convert_charrefs=convert_charrefs)
        
    def feed(self, data: str) -> None:
        # Convert data into a easier to modify type
        self.converted_data = data.splitlines()
        
        super().feed(data)
        
    def handle_starttag(self, tag: str, attrs: list) -> None:
        # Convert to dict
        attrs = dict(attrs)

        if tag == 'script' and 'src' in attrs:
            # Remove old script tag
            self.converted_data.pop(self.getpos()[0] + 1)
            
            indent = self.rawdata.splitlines()[self.getpos()[0] - 1][0] * self.getpos()[1]
            
            with open(attrs['src']) as file:
                source = textwrap.indent(file.read(), indent + '  ')
                
                # Remove src attr
                attrs.pop('src')
                
                # Add new tag
                # self.converted_data.insert(self.getpos()[0] - 1, indent + '</script>')
                
                # self.converted_data.insert(self.getpos()[0] - 1, source)
                
                # new_attrs = ' '.join([f'{key}="{value}"' for key, value in attrs.items()])
                # self.converted_data.insert(self.getpos()[0] - 1, indent + f'<script {new_attrs}>')
        elif tag == 'link':
            if 'rel' not in attrs:
                self.error(f'Attribute "rel" is missing for "<link>" in line {self.getpos()[0]}')
            if attrs['rel'] == 'stylesheet':
                # Remove old link tag
                self.converted_data.pop(self.getpos()[0] - 1)
                
                indent = self.rawdata.splitlines()[self.getpos()[0] - 1][0] * self.getpos()[1]
                
                with open(attrs['href']) as file:
                    source = textwrap.indent(file.read(), indent + '  ')
                    
                    # Remove href attr
                    attrs.pop('href')
                    
                    # Add new tag
                    self.converted_data.insert(self.getpos()[0] - 1, indent + '</style>')
                    
                    self.converted_data.insert(self.getpos()[0] - 1, source)
                    
                    new_attrs = ' '.join([f'{key}="{value}"' for key, value in attrs.items()])
                    self.converted_data.insert(self.getpos()[0] - 1, indent + f'<style {new_attrs}>')
            else:
                self.error(f'Unknown value of "rel" for "<link>" in line {self.getpos()[0]}: "{attrs["rel"]}"')

    # def handle_endtag(self, tag: str) -> None:
    #     if tag == 'script':
    #         print(f'end tag: {tag}')
    #     elif tag == 'link':
    #         print(f'end tag: {tag}')

    def error(self, message) -> None:
        print(f'{os.path.basename(sys.argv[0])}: error: {message}')
        sys.exit(1)


def main() -> None:
    '''Main function'''
    # Create argument parser
    parser = argparse.ArgumentParser(description='')

    parser.add_argument('file', help='input file', type=argparse.FileType('r'))
    parser.add_argument('-o', metavar='FILENAME', help='Write to FILENAME',
                        type=argparse.FileType('w'), default='index.h')

    args = parser.parse_args()
    packer = HTMLPacker()
    packer.feed(args.file.read())
    
    print('\n'.join(packer.converted_data))

    # Write contents of HTML file into raw string of header file
    args.o.write(DEST_STR.format('\n'.join(packer.converted_data)))


if __name__ == '__main__':
    main()
