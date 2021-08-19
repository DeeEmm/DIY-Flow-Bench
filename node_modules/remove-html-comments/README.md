#  [![NPM version][npm-image]][npm-url] [![Build Status][travis-image]][travis-url] [![Dependency Status][daviddm-image]][daviddm-url]

> Remove comments in html


## Install

```sh
$ npm install --save remove-html-comments
```


## Usage

```js
var removeHtmlComments = require('remove-html-comments');

removeHtmlComments('<!DOCTYPE html><!--[if lt IE 7]>      <html class="no-js lt-ie9 lt-ie8 lt-ie7"> <![endif]--><html lang="en"><head><meta charset="UTF-8"><title>Document</title></head><body></body></html>');
/*=> {
  data: '<!DOCTYPE html><html lang="en"><head><meta charset="UTF-8"><title>Document</title></head><body></body></html>',
  comments: [ '<!--[if lt IE 7]>      <html class="no-js lt-ie9 lt-ie8 lt-ie7"> <![endif]-->' ]
} */
```

```sh
$ npm install --global remove-html-comments
$ remove-html-comments --help

  Usage
    remove-html-comments <input-path>
    remove-html-comments <input-path> <output-path>
    cat <input-path> | remove-html-comments

  Example
    remove-html-comments index.html
    remove-html-comments index.html no-comments.html
    cat index.html | remove-html-comments
    cat index.html | remove-html-comments > no-comments.html

  Options
    -v, --verbose    Verbose output
```


## License

MIT © [Steve Mao](https://github.com/stevemao)


[npm-image]: https://badge.fury.io/js/remove-html-comments.svg
[npm-url]: https://npmjs.org/package/remove-html-comments
[travis-image]: https://travis-ci.org/stevemao/remove-html-comments.svg?branch=master
[travis-url]: https://travis-ci.org/stevemao/remove-html-comments
[daviddm-image]: https://david-dm.org/stevemao/remove-html-comments.svg?theme=shields.io
[daviddm-url]: https://david-dm.org/stevemao/remove-html-comments
