# HTTP Service Usage

The HTTP service target is `trueskill_http`. It uses `cpp-httplib` and exposes
the same operations as the CLI using JSON request and response bodies.

```sh
./build/trueskill_http --host 127.0.0.1 --port 8080
```

## Routes

- `GET /health`
- `POST /rate`
- `POST /rate-1vs1`
- `POST /quality`
- `POST /quality-1vs1`
- `POST /expose`

Request bodies use the same JSON shapes as the CLI. See [CLI usage](cli.md) for
full examples.

## Health Check

```sh
curl http://127.0.0.1:8080/health
```

Response:

```json
{"status":"ok"}
```

## Match Quality

```sh
curl -sS -H 'Content-Type: application/json' \
  -d '{"rating_groups":[[{"mu":25,"sigma":8.333333333333}],[{"mu":25,"sigma":8.333333333333}]]}' \
  http://127.0.0.1:8080/quality
```

Response:

```json
{"quality":0.4472135954999723}
```

## 1v1 Rating

```sh
curl -sS -H 'Content-Type: application/json' \
  -d '{"first_player":{"mu":25,"sigma":8.333333333333},"second_player":{"mu":25,"sigma":8.333333333333}}' \
  http://127.0.0.1:8080/rate-1vs1
```

Response:

```json
{
  "first_player": {"mu": 29.395831693, "sigma": 7.17147580701},
  "second_player": {"mu": 20.604168307, "sigma": 7.17147580701}
}
```

## Current Limits

- The server uses `cpp-httplib`, which is a blocking HTTP/1.1 library.
- It is intended for local development and early integration testing.
- It has no TLS, authentication, request logging, metrics, or concurrent worker pool yet.
