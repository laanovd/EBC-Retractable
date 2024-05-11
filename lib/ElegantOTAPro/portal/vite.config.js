import { ViteMinifyPlugin } from 'vite-plugin-minify'
import { defineConfig } from "vite"
import { viteSingleFile } from "vite-plugin-singlefile"

export default defineConfig({
	plugins: [
		ViteMinifyPlugin({}),
		viteSingleFile()
	],
})