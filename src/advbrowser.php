<?php
//error_reporting(E_ERROR | E_PARSE);

class page
{
	private $url;
	private $html;
	private $headers;
	private $curl;
	private $indent;

	public function __construct($url)
	{
		$this->url = $url;
		$this->curl = curl_init();
		curl_setopt($this->curl, CURLOPT_URL, $url);
		curl_setopt($this->curl, CURLOPT_HEADER, false);
		curl_setopt($this->curl, CURLOPT_FOLLOWLOCATION, true);
		curl_setopt($this->curl, CURLOPT_RETURNTRANSFER, true);
		$this->indent = 0;
	}

	public function fetch()
	{
		$this->html = curl_exec($this->curl);
		$this->headers = curl_getinfo($this->curl);
		//$links = $this->getLinks();
		$this->render();
		/*
		foreach($links as $num=>$link)
		{
			echo($num . ': ' . $link->textContent. "\n");
		}
		*/
	}

	public function render()
	{
		$DOM = new DOMDocument;
		$DOM->loadHTML($this->html);
		$DOM->normalizeDocument();
		$this->renderNode($DOM);
	}

	public function renderNode($node)
	{
		for($i=0; $i<$this->indent; ++$i)
		{
			echo("\t");
		}
		echo('<' . $node->nodeName);
		if($node->hasAttributes())
		{
			foreach($node->attributes as $key=>$value)
			{
				echo(' ' . $value->name. '="' . $value->value . '"');
			}
		}
		echo(">\n");
		if($node->hasChildNodes())
		{
			foreach($node->childNodes as $currentNode)
			{
				$this->indent++;
				$this->renderNode($currentNode);
				$this->indent--;
			}
		}
		for($i=0; $i<$this->indent; ++$i)
		{
			echo("\t");
		}
		echo('</' . $node->nodeName . ">\n");
	}

	private function getLinks()
	{
		$DOM = new DOMDocument;
		$DOM->loadHTML($this->html);
		return $DOM->getElementsByTagName('a');
	}
}

$currentPage = new page("http://xkcd.com");
$currentPage->fetch();
