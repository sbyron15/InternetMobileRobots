$(document).ready(function() {

    var REQUEST_PATH = '/arduino/status';

    /*$('#latency-test').on('click', function(){
        $.get('/arduino/status', function( data ) {
            $( "#latency-result" ).text( data );
            alert( "Get finished" );
        });
    });*/

    $('#latency-test').on('click', function(){
        var numRequests = parseInt($('#latency-number').val());
        results = new Array(numRequests);
        sendSequentialRequests(numRequests, results);
    });

    function sendSequentialRequests(numRequests, results){
        var start = Date.now();
        $.get(REQUEST_PATH, function(data){
            numRequests = numRequests - 1;
            results[numRequests] = Date.now() - start;
            
            if (numRequests == 0) processResults(results);
            else sendSequentialRequests(numRequests - 1, results);
        });
    }

    function processResults(results){
        var text = '';
        for (int i = 0; i < results.length; i++){
            text += results[i] + ', ';           
        }
        $('#latency-result').text(text);
    }
});
