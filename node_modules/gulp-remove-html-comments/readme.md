# gulp-remove-html-comments [![Build Status](https://travis-ci.org/stevemao/gulp-remove-html-comments.svg?branch=master)](https://travis-ci.org/stevemao/gulp-remove-html-comments)

> Remove comments in html with [remove-html-comments](https://github.com/stevemao/remove-html-comments)

*Issues with the output should be reported on the remove-html-comments [issue tracker](https://github.com/stevemao/remove-html-comments/issues).*


## Install

```
$ npm install --save-dev gulp-remove-html-comments
```


## Usage

```js
var gulp = require('gulp');
var removeHtmlComments = require('gulp-remove-html-comments');

gulp.task('default', function () {
  return gulp.src('src/*.html')
    .pipe(removeHtmlComments())
    .pipe(gulp.dest('dist'));
});
```


## License

MIT © [Steve Mao](https://github.com/stevemao)
