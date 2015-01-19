var path =require ('path') ;
var express =require ("express") ;
var recursive =require ('recursive-readdir') ;

var app =express () ;
    
app.get ("/", function (req, res) {
	res.redirect ("/index.html") ;
}) ;

app.get ("/models.js", function (req, res) {
	recursive (__dirname + '/models', [ '*.bin', '*.glsl', '*.jpg', '*.png' ], function (err, files) {
		var js ='' ; //'var sceneList =[] ;\n' ;
		for ( var i in files ) {
			var ext =path.extname (files [i]) ;
			if ( ext != '.json' && ext != '.gltf' )
				continue ;
			var name =path.basename (files [i], ext) ;
			var pathr =path.relative (__dirname, files [i]) ;
			js +="			{ \n\
				name : \"" + name + "\", \n\
				url : \"" + pathr.split (path.sep).join ("/") + "\", \n\
				cameraPos: new THREE.Vector3(30, 10, 70), \n\
				//objectScale: new THREE.Vector3(0.01, 0.01, 0.01), \n\
				//objectPosition: new THREE.Vector3(0, 1, 0), \n\
				//objectRotation: new THREE.Euler(-Math.PI / 2, 0, -Math.PI / 2), \n\
				animationTime: 3, \n\
				addLights: true, \n\
				//shadows: true, \n\
				//addGround: true \n\
			}, \n" ;
		}
		js ='var sceneList =[\n' + js + '] ;\n' ;
		res.send (js) ;
	}) ;
}) ;

app.configure (function () {
	app.use (express.methodOverride ()) ;
	app.use (express.bodyParser ()) ;
	app.use (express.static (__dirname)) ;
	app.use (express.errorHandler ({
		dumpExceptions: true, 
		showStack: true
	})) ;
	app.use (app.router) ;
}) ;

// get optional port specification
var port = 80;
if ( process.argv.length >= 3 ) {
	try {
		port =parseInt (process.argv [2]) ;
	} catch ( err ) {
		console.log ( "can't parse port number from: " + process.argv [2]) ;
	}
}

console.log( "running glTF-webgl-viewer webserver on localhost:" + port ) ;
app.listen (port) ;
