// Wrap everything in a function
(function(input) {
    var re = /.*<status>(.+)<\/status>.*/;

    switch ( input.match(re)[1] ) {
    case 'ON':
        return 'OFF';
    default: 
        return 'ON';
    }
})(input)
// input variable contains data passed by openhab