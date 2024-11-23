var gulp = require('gulp'); 
var cssnano = require('gulp-cssnano'); 
// var minifyCSS = require('gulp-minify-css');
var terser = require('gulp-terser');
const htmlmin = require('gulp-htmlmin');
var replace = require('gulp-replace');
var gzip = require('gulp-gzip');
var removeHtmlComments = require('gulp-remove-html-comments');
var clean = require('gulp-clean');
var fs = require('fs')
var json = JSON.parse(fs.readFileSync('esp32/DIY-Flow-Bench/version.json'))
console.log(json);
//console.log('GUI_BUILD_NUMBER: ' + json.GUI_BUILD_NUMBER);

gulp.task('css', function(done){    
	return gulp.src('esp32/DIY-Flow-Bench/src/style.css')       
	.pipe(cssnano())       
	// .pipe(minifyCSS())
	.pipe(gulp.dest('esp32/DIY-Flow-Bench/build'))
	done(); 
});	

gulp.task('js', function(done){    
	return gulp.src('esp32/DIY-Flow-Bench/src/javascript.js')          
	// .pipe(terser())       
	.pipe(gulp.dest('esp32/DIY-Flow-Bench/build'))
	done(); 
});

gulp.task('cssmax', function(done){    
	return gulp.src('esp32/DIY-Flow-Bench/src/style.css')       
	// .pipe(cssnano())       
	// .pipe(minifyCSS())
	.pipe(gulp.dest('esp32/DIY-Flow-Bench/build'))
	done(); 
});	

gulp.task('jsmax', function(done){    
	return gulp.src('esp32/DIY-Flow-Bench/src/javascript.js')             
	.pipe(gulp.dest('esp32/DIY-Flow-Bench/build'))
	done(); 
});

// gulp.task('html', function(done) {
// 	return gulp.src('esp32/DIY-Flow-Bench/src/index.html')
// 		.pipe(htmlmin({ collapseWhitespace: true }))
// 		.pipe(gulp.dest('esp32/DIY-Flow-Bench/build'))
// 		done();
// 	});
	
gulp.task('clean', function () {
	return gulp.src('esp32/DIY-Flow-Bench/build/*', {read: false})
	.pipe(clean());
});

gulp.task('compress+gzip', function(done) {
	gulp.src('esp32/DIY-Flow-Bench/build/index.html')
	.pipe(gzip())
	.pipe(gulp.dest('esp32/DIY-Flow-Bench/data'))
	done();
});

gulp.task('compress', function(done) {
	gulp.src('esp32/DIY-Flow-Bench/build/index.html')
	//.pipe(gzip())
	.pipe(gulp.dest('esp32/DIY-Flow-Bench/data'))
	done();
});

gulp.task('html', function() {
	return gulp.src('esp32/DIY-Flow-Bench/src/index.html')
	.pipe(replace("@@version@@", json.GUI_BUILD_NUMBER))
	.pipe(removeHtmlComments())
	.pipe(replace(/<link rel="stylesheet" type="text\/css" href="\/style.css"[^>]*>/, function(s) {
		var style = fs.readFileSync('esp32/DIY-Flow-Bench/build/style.css', 'utf8');
		return '<style>\n' + style + '\n</style>';
	}))
	.pipe(replace(/<script src="\/javascript.js"\><\/script\>/, function(s) {
		var script = fs.readFileSync('esp32/DIY-Flow-Bench/build/javascript.js', 'utf8');
		return '<script>\n' + script + '\n</script>';
	}))
	.pipe(htmlmin({ collapseWhitespace: true }))
	.pipe(gulp.dest('esp32/DIY-Flow-Bench/build'));
});	

gulp.task('htmlmax', function() {
	return gulp.src('esp32/DIY-Flow-Bench/src/index.html')
	.pipe(replace("@@version@@", json.GUI_BUILD_NUMBER))
	.pipe(replace(/<link rel="stylesheet" type="text\/css" href="\/style.css"[^>]*>/, function(s) {
		var style = fs.readFileSync('esp32/DIY-Flow-Bench/build/style.css', 'utf8');
		return '<style>\n' + style + '\n</style>';
	}))
	.pipe(replace(/<script src="\/javascript.js"\><\/script\>/, function(s) {
		var script = fs.readFileSync('esp32/DIY-Flow-Bench/build/javascript.js', 'utf8');
		return '<script>\n' + script + '\n</script>';
	}))
	.pipe(gulp.dest('esp32/DIY-Flow-Bench/build'));
});
	
gulp.task('combine', gulp.series('cssmax', 'jsmax', 'htmlmax', 'compress', 'clean'));
gulp.task('combine+minify', gulp.series('css', 'js', 'html', 'compress', 'clean'));
gulp.task('combine+minify+gzip', gulp.series('css', 'js', 'html', 'compress+gzip', 'clean'));
