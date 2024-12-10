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



function pad(num, size) {
    var s = "000000000" + num;
    return s.substring(s.length-size);
}

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
	// .pipe(replace("@@version@@", new_build_number))
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
	// .pipe(replace("@@version@@", new_build_number))
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
	





gulp.task('version', function() {


	// get release info from version.json
	gui_build_num = json.GUI_BUILD_NUMBER;
	build_num = json.BUILD_NUMBER;
	release_num = json.RELEASE;

	old_file = 'esp32/DIY-Flow-Bench/release/' + json.RELEASE + '_' + json.GUI_BUILD_NUMBER + '_index.html'

	// lets delete the old file
	fs.unlinkSync(old_file);

	// get current datetime
	const dtnow = new Date();
	year = dtnow.getFullYear().toString().substr(2);
	month = dtnow.getMonth()  + 1;
	date = dtnow.getDate();

	// create new build number
	bn_year = build_num[0] + build_num[1]
	bn_month = build_num[2] + build_num[3]
	bn_date = build_num[4] + build_num[5]
	bn_inc = build_num.slice(-4);

	console.log("yr: " + bn_year +  " mth: " + bn_month + " dte: " + bn_date + " inc: " + bn_inc);

	//  check build date incremental count
	if (bn_year == year && bn_month == month && bn_date == date) {
		incremental = Number(bn_inc)
		incremental = incremental + 1 
	} else {
		incremental = 1
	}

	// pad with leading zeroes
	inc_str = String(pad(incremental, 4))

	// Create GUI build number
	new_build_number = year + String(pad(month,2)) + String(pad(date,2)) + inc_str
	json.GUI_BUILD_NUMBER = new_build_number

	// update version.json
	fs.writeFileSync('esp32/DIY-Flow-Bench/version.json', JSON.stringify(json, null, 2) );

	// console.log(json);





	dest_file = 'esp32/DIY-Flow-Bench/release/' + json.RELEASE + '_' + new_build_number + '_index.html';

	// fs.copyFile('esp32/DIY-Flow-Bench/data/index.html', dest_file, (err) => {
	// 	if (err) throw err;
	// 	console.log('File was copied to destination');
	//   });

	fs.readFile('esp32/DIY-Flow-Bench/data/index.html', 'utf8', function (err,data) {
	if (err) return console.log(err);

	var result = data.replace("@@version@@", new_build_number);

	fs.writeFile(dest_file, result, 'utf8', function (err) {
		if (err) return console.log(err);
	});
	});

});




gulp.task('combine', gulp.series('cssmax', 'jsmax', 'htmlmax', 'compress', 'clean', 'version'));
gulp.task('combine+minify', gulp.series('css', 'js', 'html', 'compress', 'clean', 'version'));
gulp.task('combine+minify+gzip', gulp.series('css', 'js', 'html', 'compress+gzip', 'clean', 'version'));


