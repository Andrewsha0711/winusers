const http = require('http');
const fs = require('fs');
const path = require('path');
const qs = require('querystring');
const addon = require('bindings')('winapi');
const open = require('open');

const address = 'localhost';
const port = 8000;
open(`http://${address}:${port}`);

const createPath = (page) => path.resolve(__dirname, 'views', `${page}.html`);
let username;

const server = http.createServer((req, res) => {
    let bPath = '';
    switch (req.url) {
        case '/':
            bPath = createPath('form');
            res.statusCode = 200;
            switch (req.method) {
                case 'POST':
                    let body = '';
                    req.on('data', (data) => {
                        body += data;
                        if (body.length > 1e7)
                            res.end();
                    });
                    req.on('end', () => {
                        username = qs.parse(body).username;
                    });
                    res.statusCode = 301;
                    res.setHeader('Location', '/info');
                    res.end();
                    break;
                default:
                    break;
            }
            break;
        case "/info":
            if (!username) {
                res.statusCode = 301;
                res.setHeader('Location', '/');
                res.end();
            }
            else {
                bPath = createPath("info");
                res.statusCode = 200;
                let groups = addon.check(username);
                if (groups == "not found") {
                    res.write(`<h1>User with name: ${username} not found</h1>`);
                }
                else {
                    res.write(`<h1>User with name: ${username} exists and has roles:</h1>`);
                    groups = groups.split(' ')
                    groups.pop(); // remove empty string
                    groups.forEach((item) => {
                        res.write(`<h2>${item}</h2>`);
                    });
                }
            }
            break;
        default:
            res.statusCode = 404;
            break;
    }

    fs.readFile(bPath, null, (error, data) => {
        if (error) {
            res.statusCode = 500;
        }
        else {
            res.write(data);
        }
        res.end();
    })
});

server.listen(port, address, (error) => {
    error ? console.log(error) : console.log('listening');
}); 