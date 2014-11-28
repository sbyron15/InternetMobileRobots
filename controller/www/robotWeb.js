$(document).ready(function() {
    // Debug
    var DEBUG = true;
    if (!DEBUG) $('#debug').hide();

    // Constants
    var UP = 'triangle-up';
    var DOWN = 'triangle-down';
    var RIGHT = 'triangle-right';
    var LEFT = 'triangle-left';
    var MIN_SPEED = 0;
    var MAX_SPEED = 3;

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
        else if (isDirectional(e.keyCode)){
            if (isLeft(e.keyCode)) setOn(LEFT);
            else if (isUp(e.keyCode)) setOn(UP);
            else if (isRight(e.keyCode)) setOn(RIGHT);
            else if (isDown(e.keyCode)) setOn(DOWN);
            updateDirection();
        }
    }); 
    $(document).on('keyup', function(e) { 
        $('#msg').text(e.keyCode + ' key released');
        
        if (isLeft(e.keyCode)) setOff(LEFT);
        else if (isUp(e.keyCode)) setOff(UP);
        else if (isRight(e.keyCode)) setOff(RIGHT);
        else if (isDown(e.keyCode)) setOff(DOWN);
        else return; // nothing to be done

        updateDirection();
    });

    // Direction click event
    $('.arrow-box').on('click', function(){
        var id = $(this).children('.triangle').attr('id');
        
        // Update page
        isOn(id) ? setOff(id) : setOn(id);

        // Update server
        updateDirection();
    });
    
    // Direction helper functions
    function setOn(id){
        $('.triangle').removeClass('triangle-on');
        $('#' + id).addClass('triangle-on');
    }
    function setOff(id){
        $('#' + id).removeClass('triangle-on');
    }
    function isOn(id){
        return $('#' + id).hasClass('triangle-on');
    }
    
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
        $('#msg').text('Speed set to ' + $(this).val()); 
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

    // Sends direction to the server
    function updateDirection(){
        var path = '';
        var debugMsg = '';

        if (isOn(UP)){
            path = '/arduino/moveForward';
            debugMsg = 'Direction set to forward';
        }
        else if (isOn(DOWN)){
            path = '/arduino/moveBackward';
            debugMsg = 'Direction set to reverse';
        }
        else if (isOn(LEFT)){
            path = '/arduino/leftTurn';
            debugMsg = 'Direction set to left';
        }
        else if (isOn(RIGHT)){
            path = '/arduino/rightTurn';
            debugMsg = 'Direction set to right';
        }
        else { // nothing's on, make sure there is no movement
            path = '/arduino/stop';
            debugMsg = 'No direction set';
        }

        $('#direction').load(path);
        $('#msg').text(debugMsg);
    }
});
