$(document).ready(function() {
    // Debug
    var DEBUG = true;
    if (!DEBUG) $('#debug').hide();

    // Constants
    var UP = 'up';
    var DOWN = 'down';
    var RIGHT = 'right';
    var LEFT = 'left';
    var STOP = 'stop';
    var MIN_SPEED = 0;
    var MAX_SPEED = 3;

    // active direction
    var direction = STOP;
    var sending = false;
    var updating = false;

    // ensure no speed, browsers often remember previous setting
    $('#speedControl').val(MIN_SPEED);

    // Key events
    $(document).on('keydown', function(e) { 
        $('#msg').text(e.keyCode + ' key pressed'); 
        
        var sc = $('#speedControl');
        if (isSpeedUp(e.keyCode) && parseInt(sc.val()) < MAX_SPEED){
            sc.val(parseInt(sc.val()) + 1);
        }
        else if (isSlowDown(e.keyCode) && parseInt(sc.val()) > MIN_SPEED){
            sc.val(parseInt(sc.val()) - 1);
        }
        else if (isLeft(e.keyCode)) sendDirection(LEFT);
        else if (isUp(e.keyCode)) sendDirection(UP);
        else if (isRight(e.keyCode)) sendDirection(RIGHT);
        else if (isDown(e.keyCode)) sendDirection(DOWN);
    }); 
    $(document).on('keyup', function(e) { 
        $('#msg').text(e.keyCode + ' key released');
        
        if (isDirectional(e.keyCode)) {
            sendDirection(STOP);
        }
    });

    // Direction click event
    $('.direction-box').on('click', function(){
        var id = $(this).children('img').attr('id');

        if (id == direction) sendDirection(STOP);
        else sendDirection(id);
    });

    $('#clear-log').on('click', function(){
        $('#msg').load('/arduino/clearLog');
    });
    $('#force-update').on('click', function(){
        updateStatus();
    });
    
    // Keyboard helper functions
    function isLeft(keyCode){
        return keyCode == 37 || keyCode == 65;
    }
    function isRight(keyCode){
        return keyCode == 39 || keyCode == 68;
    }
    function isUp(keyCode){
        return keyCode == 38 || keyCode == 87;
    }
    function isDown(keyCode){
        return keyCode == 40 || keyCode == 83;
    }
    function isDirectional(keyCode){
        return isLeft(keyCode) || isRight(keyCode) || isUp(keyCode) || isDown(keyCode);
    }
    function isSpeedUp(keyCode){
        return keyCode == 67 || keyCode == 190;
    }
    function isSlowDown(keyCode){
        return keyCode == 88 || keyCode == 188;
    }

    // Speed change event
    $('#speedControl').on('change', function(){
        $('#msg').text('Setting speed to level ' + $(this).val()); 
        switch (parseInt($(this).val())){
            case 0: 
                $('#speed').load('/arduino/stop');
                break;
            case 1:
                $('#speed').load('/arduino/setSlowSpeed');
                break;
            case 2:
                $('#speed').load('/arduino/setMediumSpeed');
                break;
            case 3:
                $('#speed').load('/arduino/setFastSpeed');
                break;
            default:
                $('#speed').text('ERROR'); 
        }
    });

    // Sends direction to the server, page updated on response
    function sendDirection(id){
        // block consecutive call unless the it is stop
        if (sending && (id != STOP || direction == STOP)) return; 

        sending = true;

        var path = '';
        var debugMsg = '';

        if (id == UP){
            path = '/arduino/moveForward';
            debugMsg = 'Setting direction to forward';
        }
        else if (id == DOWN){
            path = '/arduino/moveBackward';
            debugMsg = 'Setting direction to reverse';
        }
        else if (id == LEFT){
            path = '/arduino/leftTurn';
            debugMsg = 'Setting direction to left';
        }
        else if (id == RIGHT){
            path = '/arduino/rightTurn';
            debugMsg = 'Setting direction to right';
        }
        else { // id is stop, or unrecognized, stop is failsafe.
            id = STOP;
            path = '/arduino/stop';
            debugMsg = 'Stopping';
        }

        $('#direction').load(path, function(){ 
            // on response, update page and state
            setDirection(id);
            sending = false;
        });

        $('#msg').text(debugMsg);
    }

    function updateStatus(){
        // don't bother updating if change will happen anyways
        if (sending || updating) return;
        updating = true;

        $('#status').text('Requesting status');

        $('#status').load('/arduino/status', function(){
            var id = STOP;
            var index = $('#status').text().indexOf(':');

            $('#msg').text('Here1');
            var d = $('#status').text().slice(0, index);

            $('#msg').text('Here2');

            var s = $('#status').text().slice(index + 1);

            $('#msg').text(d);            

            if (d == 'moveForward') id = UP;
            else if (d == 'moveBackward') id = DOWN;
            else if (d == 'leftTurn') id = LEFT;
            else if (d == 'rightTurn') id = RIGHT;

            //$('#msg').text('Here4');

            setDirection(id);
            
            $('direction').text(id);
            $('speed').text(s);

            updating = false;
        });
    }

    // set UI direction to reflect robot direction
    function setDirection(id){
        $('#' + direction).attr('src', 'img/' + direction + '_off.png');
        $('#' + id).attr('src', 'img/' + id + '_on.png');
        direction = id;
    }
});
