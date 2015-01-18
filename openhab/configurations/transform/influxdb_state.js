// Wrap everything in a function
(function(input) {
    var TEN_MINUTES = 60*10;
    var re = /"points":\[\[(\d+)/;
    var current_timestamp = Math.floor(Date.now() / 1000);
    
    var array_result = input.match(re);

    if ( !array_result || array_result[1] === undefined || !parseInt(array_result[1]) )
      return 'OFF';

    var last_influx_timestamp = parseInt(array_result[1]);

    if (current_timestamp - last_influx_timestamp > TEN_MINUTES)
      return 'OFF';

    return 'ON';

    //return input.match(re)[1];
    //return Math.floor(Date.now() / 1000);
})(input)
// input variable contains data passed by openhab