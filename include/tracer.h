/***
 * Copyright 2020 HAProxy Technologies
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#ifndef _OPENTRACING_C_WRAPPER_TRACER_H_
#define _OPENTRACING_C_WRAPPER_TRACER_H_

using TextMap = std::unordered_map<std::string, std::string>;


class TextMapCarrier : public opentracing::TextMapReader, public opentracing::TextMapWriter {
	public:
	TextMapCarrier(TextMap &text_map) : tm_data(text_map) {}

	/***
	 * TextMapWriter: Set a key:value pair to the carrier.  Multiple calls
	 * to Set() for the same key leads to undefined behavior.
	 */
	opentracing::expected<void> Set(opentracing::string_view key, opentracing::string_view value) const override
	{
		tm_data[key] = value;

		return {};
	}

	/***
	 * TextMapReader: LookupKey() returns the value for the specified
	 * key if available.  If no such key is present, it returns
	 * key_not_found_error.
	 */
	opentracing::expected<opentracing::string_view> LookupKey(opentracing::string_view key) const override
	{
		auto iter = tm_data.find(key);
		if (iter != tm_data.end())
			return opentracing::string_view{iter->second};

		return opentracing::make_unexpected(opentracing::key_not_found_error);
	}

	/***
	 * TextMapReader: ForeachKey() returns TextMap contents via repeated
	 * calls to the f() function.  If any call to f() returns an error,
	 * ForeachKey() terminates and returns that error.
	 */
	opentracing::expected<void> ForeachKey(std::function<opentracing::expected<void>(opentracing::string_view key, opentracing::string_view value)> f) const override
	{
		for (const auto &text_map : tm_data) {
			auto result = f(text_map.first, text_map.second);
			if (!result)
				return result;
		}

		return {};
	}

	private:
	TextMap &tm_data;
};


class HTTPHeadersCarrier : public opentracing::HTTPHeadersReader, public opentracing::HTTPHeadersWriter {
	public:
	HTTPHeadersCarrier(TextMap &text_map) : tm_data(text_map) {}

	/***
	 * HTTPHeadersWriter: Set a key:value pair to the carrier.  Multiple calls
	 * to Set() for the same key leads to undefined behavior.
	 */
	opentracing::expected<void> Set(opentracing::string_view key, opentracing::string_view value) const override
	{
		tm_data[key] = value;

		return {};
	}

	/***
	 * HTTPHeadersReader: LookupKey() returns the value for the specified
	 * key if available.  If no such key is present, it returns
	 * key_not_found_error.
	 */
	opentracing::expected<opentracing::string_view> LookupKey(opentracing::string_view key) const override
	{
		auto iter = tm_data.find(key);
		if (iter != tm_data.end())
			return opentracing::string_view{iter->second};

		return opentracing::make_unexpected(opentracing::key_not_found_error);
	}

	/***
	 * HTTPHeadersReader: ForeachKey() returns TextMap contents via repeated
	 * calls to the f() function.  If any call to f() returns an error,
	 * ForeachKey() terminates and returns that error.
	 */
	opentracing::expected<void> ForeachKey(std::function<opentracing::expected<void>(opentracing::string_view key, opentracing::string_view value)> f) const override
	{
		for (const auto &text_map : tm_data) {
			auto result = f(text_map.first, text_map.second);
			if (!result)
				return result;
		}

		return {};
	}

	private:
	TextMap &tm_data;
};


struct otc_tracer *ot_tracer_new(void);

#endif /* _OPENTRACING_C_WRAPPER_TRACER_H_ */

/*
 * Local variables:
 *  c-indent-level: 8
 *  c-basic-offset: 8
 * End:
 *
 * vi: noexpandtab shiftwidth=8 tabstop=8
 */
