//ctx.js - WebGL Graphics Framework - 18:40 12/06/2021
//https://developer.mozilla.org/en-US/docs/Web/JavaScript/Reference/Global_Objects/Date/now
//https://developer.mozilla.org/en-US/docs/Web/API/WebGLShader
//https://developer.mozilla.org/en-US/docs/Web/API/WebGL_API/Tutorial/Adding_2D_content_to_a_WebGL_context

var clock = null;
var request = null;

var canvas = null;
var context = null;

var vertexBuffer = null;
var attributeVertex = 0;
var vertexShader = null;
var fragmentShader = null;
var vertexShaderSource = null;
var fragmentShaderSource = null;
var shaderProgram = null;

window.onload = function()
{
    setTimeout(() => { setup(); setInterval(() => { loop(); }, 0); }, 0);
}

window.onunload = function()
{
    setTimeout(() => { cleanup(); }, 0);
}

var setup = function()
{
    ctx = ctxBegin("ctxCanvas");
    vertexBuffer = ctxCreateBuffer(ctx, [-1.0, 1.0, 1.0, 1.0, -1.0, -1.0, 1.0, -1.0]);
    vertexShader = ctxCreateShader(ctx, getFile("./shader.vertex"), ctx.VERTEX_SHADER);
    fragmentShader = ctxCreateShader(ctx, getFile("./shader.fragment"), ctx.FRAGMENT_SHADER);
    shaderProgram = ctxCreateProgram(ctx, vertexShader, fragmentShader);
}

var loop = function()
{
    ctx.clear(ctx.COLOR_BUFFER_BIT);
    ctx.useProgram(shaderProgram);
    ctx.bindBuffer(ctx.ARRAY_BUFFER, vertexBuffer);
    ctx.enableVertexAttribArray(attributeVertex);
    ctx.vertexAttribPointer(attributeVertex, 2, ctx.FLOAT, false, 0, 0);
    ctx.drawArrays(ctx.TRIANGLE_STRIP, 0, 4);
}

var cleanup = function()
{
    ctx.deleteProgram(shaderProgram);
    ctx.deleteShader(fragmentShader);
    ctx.deleteShader(vertexShader);
}

var delay = function(milliseconds)
{
    clock = Date.now();
    while ((Date.now() - clock) < milliseconds) { }
}

var getFile = function(url)
{
    request = new XMLHttpRequest();
    request.open("GET", url, false); //DO NOT SET TO TRUE!!!
    request.send(null);
    for (var i = 0; i < 100; ++i)
    {
        if (request.status == 200)
        {
            return request.responseText;
        }
        delay(10);
    }
    throw "[WARN]: getFile() failed\nReason: Timeout";
}

var ctxBegin = function(element)
{
    var context = document.getElementById(element).getContext("webgl");
    if (!context)
    {
        console.log("[WARN]: glBegin() failed\nERROR: Graphics Context Not Supported");
        return null;
    }
    context.clearColor(0.0, 0.0, 0.3, 1.0);
    context.clear(context.COLOR_BUFFER_BIT);
    return context;
}

var ctxCreateBuffer = function(context, positions)
{
    var buffer = context.createBuffer();
    context.bindBuffer(context.ARRAY_BUFFER, buffer);
    context.bufferData(context.ARRAY_BUFFER, new Float32Array(positions), context.STATIC_DRAW);
    return buffer;
}

var ctxCreateShader = function(context, source, type)
{
    var shader = context.createShader(type);
    context.shaderSource(shader, source);
    context.compileShader(shader);
    if (!context.getShaderParameter(shader, context.COMPILE_STATUS))
    {
        var info = context.getShaderInfoLog(shader);
        console.log("[WARN]: createShader() failed\n" + info);
        return null;
    }
    return shader;
}

var ctxCreateProgram = function(context, vertex, fragment)
{
    var program = context.createProgram();
    context.attachShader(program, vertex);
    context.attachShader(program, fragment);
    context.linkProgram(program);
    if (!context.getProgramParameter(program, context.LINK_STATUS))
    {
        var info = context.getProgramInfoLog(program);
        console.log("[WARN]: createProgram() failed\n" + info);
        return null;
    }
    return program;
}